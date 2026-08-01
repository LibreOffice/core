using System.Diagnostics;
using Paperless.Core.Diagnostics;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// The work one metafile decode is allowed to cause: records read, commands recorded, path
/// segments and wall-clock time.
/// </summary>
/// <remarks>
/// <para>
/// <b>Treat the stream as hostile from the first line, not after the first report.</b> A
/// metafile arrives inside a document that arrives by e-mail, and none of the three formats
/// bounds itself: the record loop is driven by a length field the file supplies, a polygon
/// carries a 16-bit point count in WMF and a 32-bit one in EMF, and nothing stops a file from
/// carrying a million records that each draw a hairline. The SVG work found two live holes in
/// a third-party parser this way — a <c>file:///etc/passwd</c> read reachable from an emailed
/// document, and a 1 057-byte input that had not finished after 120 seconds — which is why the
/// guards here exist before the first real file has been read rather than after the first bug
/// report.
/// </para>
/// <para>
/// <b>A record cap and a command cap are different caps.</b> One record can emit many commands
/// (a hatch fill emits a stroke per line) and many records emit none at all (every state
/// record). Capping only commands lets a file of ten million <c>SaveDC</c>s spin; capping only
/// records lets one polygon of four million points allocate. Both are charged, and so is the
/// clock, because a file can stay under every count and still be slow.
/// </para>
/// <para>
/// Exhaustion is not an error. It sets <see cref="IsExhausted"/>, the decoder stops, and the
/// caller gets the prefix of the picture that was decoded plus a diagnostic — the same contract
/// <c>VectorImage.IsTruncated</c> already has for SVG. Throwing would turn a hostile picture
/// into an unreadable document, which is the outcome the guard exists to prevent.
/// </para>
/// </remarks>
public sealed class MetafileBudget
{
    private readonly VectorLimits _limits;
    private readonly Stopwatch _clock = Stopwatch.StartNew();
    private int _records;
    private int _commands;
    private long _segments;
    private int _sinceClockCheck;

    /// <summary>Creates a budget from a set of limits.</summary>
    /// <param name="limits">The caps; null uses <see cref="VectorLimits.Default"/>.</param>
    public MetafileBudget(VectorLimits? limits = null) => _limits = limits ?? VectorLimits.Default;

    /// <summary>
    /// The caps a picture carried <em>inside</em> this one decodes under, or null when no
    /// nesting is left.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A metafile can carry a whole further metafile as an image, and re-entering the decoder
    /// from inside itself needs a bound the budget cannot supply: a budget is spent as work is
    /// done, and a picture nested a thousand deep that draws almost nothing at each level never
    /// spends any of it. Depth is the quantity that grows, so depth is what has to be counted —
    /// and <see cref="VectorLimits.MaxNestingDepth"/> is already the knob for it.
    /// </para>
    /// <para>
    /// The nested decode gets a budget of its own rather than sharing this one, which is the
    /// deliberate weaker half. Sharing would mean a picture drawn twice at two sizes getting a
    /// fraction of an allowance the second time and coming out as a prefix of itself — which
    /// reads as a decoding bug rather than as a limit.
    /// </para>
    /// </remarks>
    public VectorLimits? Nested => _limits.MaxNestingDepth <= 0
        ? null
        : _limits with { MaxNestingDepth = _limits.MaxNestingDepth - 1 };

    /// <summary>True once any cap has been reached, so what was decoded is a prefix.</summary>
    public bool IsExhausted { get; private set; }

    /// <summary>Why the decode stopped, or null while it has not.</summary>
    public string? Reason { get; private set; }

    /// <summary>How many records have been read.</summary>
    public int Records => _records;

    /// <summary>How many drawing commands have been recorded.</summary>
    public int Commands => _commands;

    /// <summary>Charges one record. False when the decode must stop.</summary>
    public bool ChargeRecord()
    {
        if (IsExhausted) return false;

        if (++_records > _limits.MaxRecords)
        {
            return Stop($"more than {_limits.MaxRecords} records");
        }

        // The clock is consulted every 256 records rather than every one: a Stopwatch read is
        // cheap but not free, and 256 records of the worst legitimate record type is far below
        // the granularity anyone cares about in a ten-second budget.
        if ((++_sinceClockCheck & 0xFF) == 0 && _clock.Elapsed > _limits.TimeLimit)
        {
            return Stop($"longer than {_limits.TimeLimit.TotalSeconds:0.#} s");
        }

        return true;
    }

    /// <summary>Charges one recorded drawing command. False when the decode must stop.</summary>
    public bool ChargeCommand()
    {
        if (IsExhausted) return false;

        if (++_commands > _limits.MaxCommands)
        {
            return Stop($"more than {_limits.MaxCommands} drawing commands");
        }

        if (_clock.Elapsed > _limits.TimeLimit)
        {
            return Stop($"longer than {_limits.TimeLimit.TotalSeconds:0.#} s");
        }

        return true;
    }

    /// <summary>Charges a path's segments. False when the decode must stop.</summary>
    public bool ChargeSegments(int count)
    {
        if (IsExhausted) return false;

        _segments += count;
        return _segments <= _limits.MaxPathSegments || Stop($"more than {_limits.MaxPathSegments} path segments");
    }

    /// <summary>
    /// True when a count a file states is beyond anything the remaining bytes could hold.
    /// </summary>
    /// <remarks>
    /// The cheapest guard there is, and the one that catches the commonest attack: a record that
    /// claims 400 000 points inside a 200-byte file. Checking before allocating means the
    /// allocation never happens, rather than happening and then failing.
    /// </remarks>
    /// <param name="count">The count the record states.</param>
    /// <param name="bytesEach">How many bytes one item occupies on the wire.</param>
    /// <param name="bytesAvailable">How many bytes are left in the record.</param>
    public static bool Plausible(int count, int bytesEach, long bytesAvailable)
        => count >= 0 && (long)count * bytesEach <= bytesAvailable;

    /// <summary>Records that a limit was reached and answers false, for use in a return.</summary>
    private bool Stop(string reason)
    {
        IsExhausted = true;
        Reason ??= reason;
        return false;
    }

    /// <summary>The diagnostic to raise when a decode was cut short, or null when it was not.</summary>
    public Diagnostic? Truncation(string format)
        => IsExhausted
            ? new Diagnostic(
                DiagnosticSeverity.Warning,
                "PL6032",
                $"A {format} was cut short after {Reason}; part of the picture is missing.")
            : null;
}
