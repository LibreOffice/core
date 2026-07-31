using System.Xml.Linq;

namespace Paperless.OpenDocument;

/// <summary>
/// Receives the elements a paragraph carries that mark a position or a range rather than contributing
/// text: bookmarks, change marks and fields.
/// </summary>
/// <remarks>
/// <para>
/// An interface rather than a concrete collection because what such a mark <em>means</em> is a Writer
/// concept — a bookmark's range, a redline's author — and this library sits below the word-processing
/// one in the dependency order. The content walk knows where the marks are and nothing about what to
/// do with them; the family reader knows the reverse.
/// </para>
/// <para>
/// Null by default, so a spreadsheet or a presentation pays nothing for it, and so does a text
/// document that has no marks: the walk calls this only at elements it was already stepping over.
/// </para>
/// </remarks>
public interface IOdfMarkSink
{
    /// <summary>A paragraph is about to be read.</summary>
    void OpenParagraph();

    /// <summary>A paragraph has been read, with the text it produced.</summary>
    /// <param name="text">
    /// The paragraph's own text, without any generated list label: the label is computed rather than
    /// stored, and the offsets marks are reported at do not count it.
    /// </param>
    void CloseParagraph(string text);

    /// <summary>
    /// A mark over the paragraph being read.
    /// </summary>
    /// <param name="element">
    /// The element itself, so the sink can read whichever attributes its kind carries without this
    /// library having to know them.
    /// </param>
    /// <param name="startOffset">Where it begins, as an offset into the paragraph's text so far.</param>
    /// <param name="endOffset">Where it ends. Equal to <paramref name="startOffset"/> for a point mark.</param>
    void Mark(XElement element, int startOffset, int endOffset);
}
