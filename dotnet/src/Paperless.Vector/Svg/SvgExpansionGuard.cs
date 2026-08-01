using System.Xml;
using System.Xml.Linq;

namespace Paperless.Vector.Svg;

/// <summary>
/// Refuses an SVG whose <c>&lt;use&gt;</c> references expand to more nodes than a limit
/// allows, before any of it is compiled.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is the guard that matters, and it has to run first.</b> SVG's <c>&lt;use&gt;</c>
/// is a macro: it instantiates the subtree it points at, and nothing stops thirteen nested
/// groups of three references each from expanding a 1 057-byte file into 3<sup>13</sup> ≈
/// 1.6 million drawn nodes. Measured against <c>Svg.SceneGraph</c> 5.1.1, exactly that file
/// had not finished compiling after <b>120 seconds</b>.
/// </para>
/// <para>
/// Every other cap misses it. A byte cap sees 1 KB. A command cap never fires, because the
/// explosion happens inside the library's scene compiler and no command has been emitted
/// yet. A time cap cannot help either: the loop belongs to a third-party library and .NET
/// offers no way to interrupt it. The only place to stand is in front, counting the
/// expansion arithmetically — which costs one XML parse and a memoised walk.
/// </para>
/// <para>
/// The XML is read with <c>XmlResolver = null</c> so no external entity resolves, and with a
/// character budget so an entity-expansion bomb is refused here as well.
/// </para>
/// </remarks>
internal static class SvgExpansionGuard
{
    private static readonly XName XLinkHref = XName.Get("href", "http://www.w3.org/1999/xlink");

    /// <summary>Why a document was refused, or <see cref="Verdict.Allowed"/>.</summary>
    public enum Verdict
    {
        /// <summary>Safe to hand to the parser.</summary>
        Allowed,

        /// <summary>More nodes after expansion than the limit permits.</summary>
        TooManyExpandedNodes,

        /// <summary>Deeper element nesting than the limit permits.</summary>
        TooDeep,

        /// <summary>A <c>&lt;use&gt;</c> cycle, which SVG forbids and which never terminates.</summary>
        Cyclic,
    }

    /// <summary>Decides whether a document is safe to compile.</summary>
    /// <param name="xml">The SVG source.</param>
    /// <param name="limits">The caps to apply.</param>
    /// <returns>
    /// The verdict. XML this cannot parse is <see cref="Verdict.Allowed"/>: the library's
    /// parser is deliberately more forgiving than a conforming reader, and refusing every
    /// document it could still have read would trade a real capability for no extra safety —
    /// the byte cap still bounds what such a document can be.
    /// </returns>
    public static Verdict Inspect(string xml, VectorLimits limits)
    {
        ArgumentNullException.ThrowIfNull(limits);

        XDocument document;
        try
        {
            using StringReader text = new(xml);
            using XmlReader reader = XmlReader.Create(text, new XmlReaderSettings
            {
                DtdProcessing = DtdProcessing.Parse,
                XmlResolver = null,
                MaxCharactersFromEntities = 64 * 1024,
                MaxCharactersInDocument = limits.MaxBytes,
                IgnoreComments = true,
                IgnoreProcessingInstructions = true,
                IgnoreWhitespace = true,
            });

            document = XDocument.Load(reader);
        }
        catch (Exception exception) when (exception is XmlException or InvalidOperationException or IOException)
        {
            return Verdict.Allowed;
        }

        if (document.Root is not { } root) return Verdict.Allowed;

        Dictionary<string, XElement> byId = [];
        foreach (XElement element in root.DescendantsAndSelf())
        {
            if (element.Attribute("id")?.Value is { Length: > 0 } id) byId.TryAdd(id, element);
        }

        Dictionary<XElement, long> costs = [];
        HashSet<XElement> active = [];
        bool cyclic = false;

        long total = Cost(root, byId, costs, active, limits, ref cyclic, 0, out bool tooDeep);

        if (cyclic) return Verdict.Cyclic;
        if (tooDeep) return Verdict.TooDeep;

        return total > limits.MaxExpandedNodes ? Verdict.TooManyExpandedNodes : Verdict.Allowed;
    }

    /// <summary>
    /// How many nodes an element becomes once its references are instantiated.
    /// </summary>
    /// <remarks>
    /// Saturating rather than exact: once the running total passes the limit the answer is
    /// only used to refuse, so there is no reason to keep multiplying — and every reason not
    /// to, since the true figure is what overflows.
    /// </remarks>
    private static long Cost(
        XElement element,
        Dictionary<string, XElement> byId,
        Dictionary<XElement, long> costs,
        HashSet<XElement> active,
        VectorLimits limits,
        ref bool cyclic,
        int depth,
        out bool tooDeep)
    {
        tooDeep = false;

        if (depth > limits.MaxNestingDepth)
        {
            tooDeep = true;
            return 1;
        }

        if (costs.TryGetValue(element, out long cached)) return cached;

        if (!active.Add(element))
        {
            cyclic = true;
            return 1;
        }

        long total = 1;

        if (element.Name.LocalName is "use" && Reference(element) is { } target
            && byId.TryGetValue(target, out XElement? referenced))
        {
            total += Cost(referenced, byId, costs, active, limits, ref cyclic, depth + 1, out bool deeper);
            tooDeep |= deeper;
        }

        foreach (XElement child in element.Elements())
        {
            total += Cost(child, byId, costs, active, limits, ref cyclic, depth + 1, out bool deeper);
            tooDeep |= deeper;

            if (total > limits.MaxExpandedNodes)
            {
                total = limits.MaxExpandedNodes + 1L;
                break;
            }
        }

        active.Remove(element);
        costs[element] = total;
        return total;
    }

    /// <summary>The fragment id a <c>&lt;use&gt;</c> points at, if it points inside the document.</summary>
    /// <remarks>
    /// SVG 2 spells it <c>href</c> and SVG 1.1 spells it <c>xlink:href</c>; producers still
    /// write both, so both are read. Anything that is not a bare fragment is a reference out
    /// of the document, which nothing here will resolve anyway.
    /// </remarks>
    private static string? Reference(XElement element)
    {
        string? value = element.Attribute("href")?.Value ?? element.Attribute(XLinkHref)?.Value;

        return value is { Length: > 1 } text && text[0] == '#' ? text[1..] : null;
    }
}
