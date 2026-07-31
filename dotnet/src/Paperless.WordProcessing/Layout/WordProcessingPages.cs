using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// The pages a word-processing document laid out to.
/// </summary>
/// <remarks>
/// The adapter between the paginator's own result and the family-neutral
/// <see cref="IPageSequence"/> a caller asks for. The two are kept apart because the paginator's page
/// carries the line boxes a renderer needs and a spreadsheet's or a slide's does not — there is nothing
/// useful in an abstraction that covers all three beyond a count and a size.
/// </remarks>
public sealed class WordProcessingPages : IPageSequence
{
    private readonly List<PageView> _pages;

    internal WordProcessingPages(
        IReadOnlyList<LaidOutPage> pages, IReadOnlyList<PageParagraph>? paragraphs = null)
    {
        ArgumentNullException.ThrowIfNull(pages);
        Paragraphs = paragraphs ?? [];
        _pages = [.. pages.Select(page => new PageView(page, Paragraphs))];
    }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>The laid-out pages, with the line boxes a renderer needs.</summary>
    public IReadOnlyList<LaidOutPage> Pages => [.. _pages.Select(page => page.Laid)];

    /// <summary>
    /// The paragraphs the pages were filled from, indexed by <see cref="PlacedLine.ParagraphIndex"/>.
    /// </summary>
    /// <remarks>
    /// Carried rather than left to the caller to rebuild, because a <see cref="PlacedLine"/> holds a
    /// character range and not the characters: pagination places lines and leaves the strings where they
    /// were. Anything reading a page back — a renderer, or a test comparing words — needs both halves,
    /// and rebuilding the list means re-resolving every style and re-reading every font.
    /// </remarks>
    public IReadOnlyList<PageParagraph> Paragraphs { get; }

    /// <summary>
    /// The text of a placed line.
    /// </summary>
    /// <remarks>
    /// Empty when the line's paragraph is out of range, which happens only if a caller built this without
    /// its paragraphs — worth answering rather than throwing, since a page whose text cannot be recovered
    /// is still a page with a size and a body area.
    /// </remarks>
    public string TextOf(PlacedLine line)
    {
        if (line.ParagraphIndex < 0 || line.ParagraphIndex >= Paragraphs.Count) return string.Empty;

        string paragraph = Paragraphs[line.ParagraphIndex].Text;
        return line.Box.Line.VisibleTextIn(paragraph).ToString();
    }

    private sealed class PageView(LaidOutPage laid, IReadOnlyList<PageParagraph> paragraphs) : IPage
    {
        internal LaidOutPage Laid { get; } = laid;

        /// <inheritdoc/>
        public int Index => Laid.Index;

        /// <inheritdoc/>
        public DocSize Size => Laid.Size;

        /// <inheritdoc/>
        /// <remarks>
        /// The printed page number, which is not the index: a section can restart numbering, so a title
        /// page may be numbered zero and the page after it one.
        /// </remarks>
        public string? Label => Laid.Number.ToString(System.Globalization.CultureInfo.InvariantCulture);

        /// <inheritdoc/>
        /// <remarks>
        /// The page's body text, as one glyph run per line for a uniform paragraph and one per formatting
        /// change for a mixed one. Not its furniture: headers, footers and floating frames are not placed
        /// by pagination yet, so what is drawn is what the page holds.
        /// </remarks>
        public void Draw(IDrawingSink sink) => PageDrawing.Draw(Laid, paragraphs, sink);
    }
}
