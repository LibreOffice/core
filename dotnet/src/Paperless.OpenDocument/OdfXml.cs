using System.Xml;
using System.Xml.Linq;

namespace Paperless.OpenDocument;

/// <summary>XML loading rules shared by every ODF part.</summary>
public static class OdfXml
{
    /// <summary>
    /// Creates a reader configured for untrusted ODF XML.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="DtdProcessing.Prohibit"/> and a null resolver are the security-relevant
    /// part: office documents are untrusted input, and an external DTD or entity reference
    /// would otherwise be an XXE and SSRF vector.
    /// </para>
    /// <para>
    /// Whitespace is <em>not</em> ignored, and that is deliberate. A whitespace-only text
    /// node between two elements is real document text in ODF — the space in
    /// <c>&lt;text:span&gt;one&lt;/text:span&gt; &lt;text:span&gt;two&lt;/text:span&gt;</c>
    /// is a whitespace-only node, and dropping it would silently run the two words together.
    /// ODF's own white-space collapsing rules are applied later, when the text is read, where
    /// there is enough context to apply them correctly.
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
    /// Loads an ODF part into an <see cref="XDocument"/>, or returns null when it cannot be
    /// parsed at all.
    /// </summary>
    /// <remarks>
    /// A whole-document tree rather than a streaming parse: ODF resolution is inherently
    /// non-local — a run's formatting depends on styles declared in a different file, and a
    /// list label depends on counters from earlier siblings — so a streaming reader would
    /// need to buffer most of the document anyway. Package limits already bound how much can
    /// be read (see <c>ZipPackageBase.Limits</c>).
    /// </remarks>
    public static XDocument? TryLoad(Stream input, out string? error)
    {
        ArgumentNullException.ThrowIfNull(input);
        try
        {
            using XmlReader reader = CreateSafeReader(input);
            error = null;
            return XDocument.Load(reader, LoadOptions.None);
        }
        catch (XmlException ex)
        {
            error = ex.Message;
            return null;
        }
    }
}
