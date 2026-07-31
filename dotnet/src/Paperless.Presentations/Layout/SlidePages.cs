using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Presentations.Layout;

/// <summary>
/// The pages a deck lays out to: one per slide, in presentation order.
/// </summary>
/// <remarks>
/// <para>
/// The presentation family's answer to "what is a page", and the simplest of the three: a deck
/// states its slide size once and every slide is that size, so pagination has nothing to decide.
/// What a word processor's paginator spends its time on — where the content breaks — has no
/// equivalent here, which is why this is an adapter and not an engine.
/// </para>
/// <para>
/// <strong>A hidden slide is laid out and is not a page.</strong> The two halves of that pull in
/// opposite directions and both are settled by measurement: extraction reports a hidden slide and
/// flags it, because a caller indexing a corpus wants its words, while LibreOffice's own PDF
/// export leaves it out — <c>deck-features.pptx</c> has four slides and its reference PDF has
/// three pages. So <see cref="Slides"/> holds every slide the deck states, in the deck's own
/// order, and the page sequence holds the shown ones. A renderer that produced a page per slide
/// would disagree with the reference on the page <em>count</em> of every deck with a hidden
/// slide, which is a difference no per-shape comparison notices.
/// </para>
/// <para>
/// A hidden slide still consumes a slide <em>number</em>: LibreOffice's page numbers come from
/// the position in <c>SdPage</c>'s own list, which a hidden page stays in, and a deck's
/// <c>a:fld</c> slide-number fields carry the cached value the authoring application computed the
/// same way. So skipping a hidden page here does not renumber the ones after it.
/// </para>
/// </remarks>
public sealed class SlidePages : IPageSequence
{
    private readonly List<LaidOutSlide> _slides;
    private readonly List<SlideView> _pages;

    /// <summary>Creates a sequence over slides that have been laid out.</summary>
    /// <param name="slides">The slides, in presentation order, hidden ones included.</param>
    public SlidePages(IReadOnlyList<LaidOutSlide> slides)
    {
        ArgumentNullException.ThrowIfNull(slides);

        _slides = [.. slides];
        _pages = [];

        foreach (LaidOutSlide slide in _slides)
        {
            if (slide.IsHidden) continue;
            _pages.Add(new SlideView(slide, _pages.Count));
        }
    }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>
    /// Every laid-out slide, hidden ones included, with the shapes a renderer or a test needs.
    /// </summary>
    /// <remarks>
    /// Indexed by the deck's own slide order rather than by page, so a test naming
    /// <c>Slides[3]</c> names the deck's fourth slide whether or not the third is shown.
    /// </remarks>
    public IReadOnlyList<LaidOutSlide> Slides => _slides;

    private sealed class SlideView(LaidOutSlide slide, int index) : IPage
    {
        internal LaidOutSlide Slide { get; } = slide;

        /// <inheritdoc/>
        /// <remarks>
        /// The position in the page sequence, not the slide's position in the deck: the two differ
        /// by every hidden slide before it, and a page's index has to agree with the index a caller
        /// asked for it by.
        /// </remarks>
        public int Index => index;

        /// <inheritdoc/>
        public DocSize Size => Slide.Size;

        /// <inheritdoc/>
        /// <remarks>
        /// The slide's own name when it has one, which PresentationML and ODF both make optional
        /// — LibreOffice's PPTX export writes none at all. Manufacturing "Slide 3" would claim
        /// the file said something it did not, so an unnamed slide has a null label and the
        /// index remains the way to refer to it.
        /// </remarks>
        public string? Label => Slide.Name;

        /// <inheritdoc/>
        public void Draw(IDrawingSink sink) => SlideDrawing.Draw(Slide, sink);
    }
}
