using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>Which set operation combines two rectangle sets.</summary>
internal enum RegionOp
{
    /// <summary>Everything in either set.</summary>
    Union,

    /// <summary>Everything in both sets.</summary>
    Intersect,

    /// <summary>Everything in the left set and not in the right.</summary>
    Difference,

    /// <summary>Everything in exactly one of the two sets.</summary>
    SymmetricDifference,
}

/// <summary>
/// Exact boolean algebra over sets of axis-aligned rectangles.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is the whole of what a GDI region is.</b> An <c>EMR_EXTSELECTCLIPRGN</c> carries an
/// <c>RGNDATA</c>, which is by definition a scan list of rectangles, and every one of its five
/// combine modes — copy, and, or, xor, diff — maps onto an operation performed here exactly.
/// So the record that looked as though it needed a general polygon boolean needs nothing of the
/// sort: rectangle sets are <em>closed</em> under all four operations below, and every edge of
/// a result is an edge of an operand. What is left over is an arbitrary <em>path</em> as an
/// operand — <c>SelectClipPath</c> with <c>RGN_XOR</c>, EMF+ <c>SetClipPath</c> with a union or
/// a complement — and <c>src/Paperless.Vector/TODO.md</c> records how often that occurs in real
/// files and why the general boolean is declined.
/// </para>
/// <para>
/// <b>The algorithm is a y-band sweep, and the canonical form it produces is the point.</b>
/// Every distinct top and bottom edge of either operand cuts the plane into horizontal bands;
/// within one band every rectangle either spans it completely or misses it entirely, so the
/// band reduces to a one-dimensional problem over x intervals, where all four operations are a
/// two-cursor merge of sorted disjoint lists. Bands with identical interval lists are then
/// coalesced vertically. The result is disjoint, sorted and independent of the order the
/// operands were written in — which matters because <see cref="MetafileClip"/> feeds it straight
/// back in as the left operand of the next combine. The naive alternative, subtracting one
/// rectangle at a time, is what this replaced: it quadruples the count per exclusion and hits a
/// thousand rectangles after five of them, on a picture whose clip is two rectangles.
/// </para>
/// <para>
/// Coordinates are <see cref="Length"/>, so every comparison is exact integer arithmetic and no
/// tolerance appears anywhere. That is the second reason a rectangle boolean is worth having
/// where a Bézier one is not: a crossover solver over flattened curves has to decide when two
/// nearly coincident edges are the same edge, and this never asks the question.
/// </para>
/// </remarks>
internal static class RectangleRegion
{
    /// <summary>Combines two rectangle sets, exactly.</summary>
    /// <param name="left">The left operand; need not be disjoint or sorted.</param>
    /// <param name="right">The right operand; need not be disjoint or sorted.</param>
    /// <param name="op">Which set operation to perform.</param>
    /// <returns>Disjoint, sorted rectangles covering exactly the result.</returns>
    public static DocRect[] Combine(IReadOnlyList<DocRect> left, IReadOnlyList<DocRect> right, RegionOp op)
    {
        ArgumentNullException.ThrowIfNull(left);
        ArgumentNullException.ThrowIfNull(right);

        // Both operands cut bands even for an intersection, because a rectangle of one may start
        // part-way down a rectangle of the other and the band has to be split there.
        List<Length> edges = [];
        foreach (DocRect rect in left) AddEdges(edges, rect);
        foreach (DocRect rect in right) AddEdges(edges, rect);

        if (edges.Count < 2) return [];

        edges.Sort();

        // The two operands are walked with an active set rather than rescanned per band, which is
        // what keeps a file of four hundred ExcludeClipRect records linear rather than cubic.
        // Rescanning is the obvious implementation and it is fast enough on any picture and far
        // too slow on an adversarial one: four hundred exclusions build eight hundred bands over
        // a thousand rectangles, and the decode ran past its own wall-clock cap and returned an
        // empty picture — which reads as a clip bug rather than as a complexity one.
        DocRect[] byTopLeft = Sorted(left);
        DocRect[] byTopRight = Sorted(right);
        List<DocRect> activeLeft = [];
        List<DocRect> activeRight = [];
        int nextLeft = 0, nextRight = 0;

        List<DocRect> result = [];
        List<Span> previous = [];
        List<Span> leftSpans = [];
        List<Span> rightSpans = [];
        List<Span> current = [];

        Length bandTop = Length.Zero;
        Length bandBottom = Length.Zero;
        bool open = false;

        for (int i = 0; i + 1 < edges.Count; i++)
        {
            Length top = edges[i];
            Length bottom = edges[i + 1];
            if (bottom <= top) continue;

            Advance(byTopLeft, ref nextLeft, activeLeft, top);
            Advance(byTopRight, ref nextRight, activeRight, top);

            Spans(activeLeft, leftSpans);
            Spans(activeRight, rightSpans);
            Merge(leftSpans, rightSpans, op, current);

            // A band repeating the one above it is the same rectangles one row taller, so it is
            // held open rather than emitted. That is what keeps a plain rectangle a single
            // rectangle however many edges of the other operand cut through it.
            if (open && bandBottom == top && Same(previous, current))
            {
                bandBottom = bottom;
                continue;
            }

            if (open) Emit(result, previous, bandTop, bandBottom);

            previous.Clear();
            previous.AddRange(current);
            bandTop = top;
            bandBottom = bottom;
            open = current.Count > 0;
        }

        if (open) Emit(result, previous, bandTop, bandBottom);

        return [.. result];
    }

    /// <summary>The canonical disjoint form of a rectangle set.</summary>
    public static DocRect[] Normalise(IReadOnlyList<DocRect> rectangles)
        => Combine(rectangles, [], RegionOp.Union);

    private static void AddEdges(List<Length> edges, DocRect rect)
    {
        if (rect.IsEmpty) return;
        edges.Add(rect.Top);
        edges.Add(rect.Bottom);
    }

    /// <summary>The non-empty rectangles, ordered by their top edge.</summary>
    private static DocRect[] Sorted(IReadOnlyList<DocRect> rectangles)
    {
        List<DocRect> kept = new(rectangles.Count);
        foreach (DocRect rect in rectangles) if (!rect.IsEmpty) kept.Add(rect);

        DocRect[] order = [.. kept];
        Array.Sort(order, static (a, b) => a.Top.Emu.CompareTo(b.Top.Emu));
        return order;
    }

    /// <summary>
    /// Brings the active set up to a band's top edge.
    /// </summary>
    /// <remarks>
    /// Because the bands are cut at every horizontal edge of both operands, a rectangle either
    /// spans a band completely or misses it — so "started at or above this band and not yet
    /// ended" is exactly the membership test, and no rectangle has to be looked at twice.
    /// </remarks>
    private static void Advance(DocRect[] sorted, ref int next, List<DocRect> active, Length top)
    {
        while (next < sorted.Length && sorted[next].Top <= top) active.Add(sorted[next++]);
        active.RemoveAll(rect => rect.Bottom <= top);
    }

    /// <summary>
    /// The x intervals the active rectangles cover, merged and sorted.
    /// </summary>
    /// <remarks>
    /// One dimension, which is the whole point of cutting bands: within a band every active
    /// rectangle spans it top to bottom, so only its left and right edges matter.
    /// </remarks>
    private static void Spans(List<DocRect> rectangles, List<Span> into)
    {
        into.Clear();

        foreach (DocRect rect in rectangles) into.Add(new Span(rect.Left, rect.Right));

        if (into.Count <= 1) return;

        into.Sort(static (a, b) => a.Start.Emu.CompareTo(b.Start.Emu));

        int write = 0;
        for (int read = 1; read < into.Count; read++)
        {
            if (into[read].Start <= into[write].End)
            {
                into[write] = new Span(into[write].Start, Length.Max(into[write].End, into[read].End));
            }
            else
            {
                into[++write] = into[read];
            }
        }

        into.RemoveRange(write + 1, into.Count - write - 1);
    }

    /// <summary>
    /// The set operation over two sorted, disjoint interval lists, in one linear sweep.
    /// </summary>
    /// <remarks>
    /// All four operations go through the same sweep rather than being composed out of
    /// difference and union, which is what keeps symmetric difference exact: composing it would
    /// produce two lists meeting along a zero-width seam, and the merge would then have to
    /// decide whether to close it.
    /// </remarks>
    private static void Merge(List<Span> left, List<Span> right, RegionOp op, List<Span> into)
    {
        into.Clear();

        if (left.Count == 0 && right.Count == 0) return;

        int i = 0, j = 0;
        Length x = left.Count == 0 ? right[0].Start
            : right.Count == 0 ? left[0].Start
            : Length.Min(left[0].Start, right[0].Start);

        bool pending = false;
        Length start = Length.Zero;

        while (true)
        {
            while (i < left.Count && left[i].End <= x) i++;
            while (j < right.Count && right[j].End <= x) j++;
            if (i >= left.Count && j >= right.Count) break;

            bool inLeft = i < left.Count && left[i].Start <= x;
            bool inRight = j < right.Count && right[j].Start <= x;

            bool wanted = op switch
            {
                RegionOp.Union => inLeft || inRight,
                RegionOp.Intersect => inLeft && inRight,
                RegionOp.Difference => inLeft && !inRight,
                _ => inLeft ^ inRight,
            };

            // The next place membership can change: the end of whichever interval the sweep is
            // inside, or the start of whichever it has not reached.
            Length? next = null;
            if (i < left.Count) next = inLeft ? left[i].End : left[i].Start;
            if (j < right.Count)
            {
                Length candidate = inRight ? right[j].End : right[j].Start;
                next = next is { } n ? Length.Min(n, candidate) : candidate;
            }

            if (next is not { } stop || stop <= x) break;

            if (wanted && !pending) { start = x; pending = true; }
            else if (!wanted && pending) { if (x > start) into.Add(new Span(start, x)); pending = false; }

            x = stop;
        }

        if (pending && x > start) into.Add(new Span(start, x));
    }

    private static bool Same(List<Span> a, List<Span> b)
    {
        if (a.Count != b.Count) return false;
        for (int i = 0; i < a.Count; i++) if (a[i] != b[i]) return false;
        return true;
    }

    private static void Emit(List<DocRect> into, List<Span> spans, Length top, Length bottom)
    {
        if (bottom <= top) return;
        foreach (Span span in spans) into.Add(new DocRect(span.Start, top, span.End - span.Start, bottom - top));
    }

    private readonly record struct Span(Length Start, Length End);
}
