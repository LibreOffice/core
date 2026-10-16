using System.Xml;
using System.Xml.Linq;

namespace XRay.Office.Ooxml;

/// <summary>
/// Loading rules shared by every OOXML part: safe parsing, strict-versus-transitional
/// namespaces, and Markup Compatibility.
/// </summary>
/// <remarks>
/// Both normalisations happen once, at load. The alternative — checking two namespaces and
/// unwrapping <c>mc:AlternateContent</c> at every point that walks the tree — is the version
/// of this that gets forgotten in one place, and the symptom is content that silently appears
/// twice or not at all.
/// </remarks>
public static class OoxmlXml
{
    /// <summary>
    /// Creates a reader configured for untrusted OOXML.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="DtdProcessing.Prohibit"/> and a null resolver are the security-relevant part:
    /// office documents are untrusted input, and an external DTD or entity reference would
    /// otherwise be an XXE and an SSRF vector.
    /// </para>
    /// <para>
    /// Whitespace is not ignored. A run's text is <c>w:t</c> content, and
    /// <c>xml:space="preserve"</c> on it means the spaces are real; discarding whitespace-only
    /// nodes would run words together at every run boundary.
    /// </para>
    /// </remarks>
    public static XmlReader CreateSafeReader(Stream input)
    {
        ArgumentNullException.ThrowIfNull(input);
        return XmlReader.Create(input, new XmlReaderSettings
        {
            DtdProcessing = DtdProcessing.Prohibit,
            XmlResolver = null,
            IgnoreWhitespace = false,
            IgnoreComments = true,
            IgnoreProcessingInstructions = true,
            CloseInput = false,
        });
    }

    /// <summary>
    /// Loads and normalises an OOXML part, or returns null when it cannot be parsed.
    /// </summary>
    /// <param name="input">The part's bytes.</param>
    /// <param name="error">The parser's message when the part could not be read.</param>
    public static XElement? TryLoad(Stream input, out string? error)
    {
        ArgumentNullException.ThrowIfNull(input);
        try
        {
            using XmlReader reader = CreateSafeReader(input);
            XDocument document = XDocument.Load(reader, LoadOptions.None);
            error = null;
            if (document.Root is null) return null;

            Normalise(document.Root);
            return document.Root;
        }
        catch (XmlException exception)
        {
            error = exception.Message;
            return null;
        }
    }

    /// <summary>
    /// Rewrites strict namespaces to transitional and resolves <c>mc:AlternateContent</c> in
    /// place.
    /// </summary>
    /// <remarks>
    /// Both are pure simplifications of the tree: afterwards, every element is named the way
    /// the transitional schema names it, and no Markup Compatibility element remains, so the
    /// code that walks the content can be written once against one vocabulary.
    /// </remarks>
    public static void Normalise(XElement root)
    {
        ArgumentNullException.ThrowIfNull(root);
        RewriteStrictNamespaces(root);
        ResolveAlternateContent(root);
    }

    private static void RewriteStrictNamespaces(XElement root)
    {
        // Depth-first over a materialised list: renaming an element while enumerating its
        // parent's children is not safe.
        foreach (XElement element in root.DescendantsAndSelf().ToList())
        {
            if (OoxmlNamespaces.StrictToTransitional.TryGetValue(
                    element.Name.NamespaceName, out string? transitional))
            {
                element.Name = XName.Get(element.Name.LocalName, transitional);
            }

            foreach (XAttribute attribute in element.Attributes().ToList())
            {
                if (attribute.IsNamespaceDeclaration) continue;
                if (!OoxmlNamespaces.StrictToTransitional.TryGetValue(
                        attribute.Name.NamespaceName, out string? attributeNamespace))
                    continue;

                // An attribute cannot be renamed, so it is replaced.
                string value = attribute.Value;
                attribute.Remove();
                element.SetAttributeValue(XName.Get(attribute.Name.LocalName, attributeNamespace), value);
            }
        }
    }

    /// <summary>
    /// Replaces each <c>mc:AlternateContent</c> with the best branch it offers.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The element exists so that a producer can write the same content twice: a
    /// high-fidelity <c>mc:Choice</c> for readers that understand some extension, and an
    /// <c>mc:Fallback</c> for those that do not. A reader that walks both gets the content
    /// twice — a text box's text appearing twice in the extracted text is the usual symptom —
    /// and one that walks neither loses it entirely.
    /// </para>
    /// <para>
    /// A choice is preferred when every namespace its <c>Requires</c> names is one XRay.Office
    /// can actually read, because the fallback exists precisely for the case where the choice
    /// cannot be.
    /// </para>
    /// <para>
    /// The one exception is a fallback that is not content. Excel writes an extended
    /// ("chartex") chart as a choice beside a generated rectangle whose only text tells the
    /// reader their Excel is too old; taking that fallback draws the notice instead of the
    /// chart. There the choice wins even though it cannot be drawn — see
    /// <c>IsAdvisoryPlaceholderFallback</c>, which is keyed on the chartex graphic-data URI and
    /// deliberately does not generalise to the identically-shaped slicer placeholder, because
    /// the reference draws that one.
    /// </para>
    /// </remarks>
    private static void ResolveAlternateContent(XElement root)
    {
        XName alternateContent = XName.Get("AlternateContent", OoxmlNamespaces.MarkupCompatibility);

        // Outermost first, and re-queried each time: resolving one may expose another nested
        // inside the branch that was chosen.
        while (root.DescendantsAndSelf(alternateContent).FirstOrDefault() is { } element)
        {
            XElement? chosen = null;
            foreach (XElement choice in element.Elements(
                         XName.Get("Choice", OoxmlNamespaces.MarkupCompatibility)))
            {
                if (IsUnderstood(choice) || IsAdvisoryPlaceholderFallback(element, choice))
                {
                    chosen = choice;
                    break;
                }
            }
            chosen ??= element.Element(XName.Get("Fallback", OoxmlNamespaces.MarkupCompatibility));

            if (chosen is null)
            {
                element.Remove();
                continue;
            }

            element.ReplaceWith(chosen.Nodes().ToList());
        }

        // A choice we cannot read still wins when the fallback beside it is not content but a
        // notice addressed to a human running an older Excel.
        //
        // The case that forced this is the extended ("chartex") chart. Excel writes the chart in
        // a Requires="cx1" choice and, beside it, a generated rectangle holding the sentence
        // "This chart isn't available in your version of Excel. Editing this shape or saving this
        // workbook into a different file format will permanently break the chart." Taking the
        // fallback — which is what a reader that cannot draw the choice is otherwise supposed to
        // do — puts 26 words of English advice onto the page where the chart should be, and they
        // land in the extracted text of every consumer downstream. LibreOffice 26.2.4.2 draws the
        // choice; measured over both corpus witnesses, its rendering contains none of that
        // sentence.
        //
        // Deliberately narrow. It is keyed on the chartex graphic-data URI, not on the prose and
        // not on the shape's shape: the same advisory pattern is used for slicers
        // (Requires="a14"/"sle15"), and there the reference *does* draw the fallback, so
        // suppressing advisories in general would be wrong. Censused prefix-agnostically over the
        // 946-document corpus, the chartex URI occurs in two documents, both spreadsheets.
        //
        // A slicer needs no rule of its own any more, and this is where the one it had used to
        // sit. It was keyed on the slicer graphic-data URI and existed because `a14` was in
        // `UnderstoodExtensions`, so an `a14` slicer choice was taken and drew nothing where the
        // reference draws its advisory rectangle. `a14` is no longer understood — `oox` and
        // writerfilter both refuse it, see `OoxmlNamespaces.UnderstoodExtensions` — so the general
        // rule now reaches every one of the corpus's seven `a14` slicer choices, and the special
        // case was unreachable. `cx1` is genuinely different: `oox` *does* support it, which is why
        // the chartex clause above is a real exception and the slicer one never was.
        //
        // The frame is drawn by `DrawingChartex`, so preferring the choice is now right about the
        // ink as well as about the text. Before that reader existed the frame drew empty, and the
        // preference was still right for the narrower reason above: both answers were wrong about
        // the ink and only one invented twenty-six words.
        static bool IsAdvisoryPlaceholderFallback(XElement element, XElement choice)
        {
            if (element.Element(XName.Get("Fallback", OoxmlNamespaces.MarkupCompatibility)) is null)
                return false;

            foreach (XElement data in choice.Descendants(
                         XName.Get("graphicData", OoxmlNamespaces.DrawingML)))
            {
                if (data.Attribute("uri")?.Value == OoxmlNamespaces.ExtendedChart) return true;
            }
            return false;
        }

        static bool IsUnderstood(XElement choice)
        {
            string? requires = choice.Attribute("Requires")?.Value;
            if (string.IsNullOrWhiteSpace(requires)) return true;

            foreach (string prefix in requires.Split(
                         ' ', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
            {
                // Requires names prefixes, not URIs, and the prefix is only meaningful through
                // the declarations in scope — so it has to be resolved against the element.
                XNamespace? resolved = choice.GetNamespaceOfPrefix(prefix);
                if (resolved is null
                    || !OoxmlNamespaces.UnderstoodExtensions.Contains(resolved.NamespaceName))
                    return false;
            }
            return true;
        }
    }
}
