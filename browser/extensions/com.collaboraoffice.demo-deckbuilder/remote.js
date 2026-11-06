remote.buildSlides = function buildSlides(slides, theme) {
  const p = SlidesApp.getActivePresentation();
  const pageWidth = p.getPageWidth();
  const pageHeight = p.getPageHeight();
  const margin = pageWidth * 0.08;
  const contentWidth = pageWidth - 2 * margin;
  const titleTop = pageHeight * 0.10;
  const titleHeight = pageHeight * 0.18;
  const bodyTop = titleTop + titleHeight + pageHeight * 0.05;
  const bodyHeight = pageHeight - bodyTop - pageHeight * 0.08;
  const titleSize = Math.round(pageHeight * 0.08);
  const bodySize = Math.round(pageHeight * 0.045);
  // A run with no text is skipped: it would produce an empty range, and an empty range has no
  // text style.
  function appendRuns(text, runs) {
    for (const run of runs) {
      if (run.text.length === 0) continue;
      const style = text.appendText(run.text).getTextStyle();
      if (run.bold) style.setBold(true);
      if (run.italic) style.setItalic(true);
      if (run.strike) style.setStrikethrough(true);
    }
  }
  let built = 0;
  for (const s of slides) {
    const slide = p.appendSlide().setBackgroundColor(theme.background);
    if (s.title.length > 0) {
      const text = slide.insertTextBox('', margin, titleTop, contentWidth,
        titleHeight).getText();
      appendRuns(text, s.title);
      // The whole title is bold by design, on top of any inline styling.  A title whose runs
      // were all empty leaves the box without text, and then there is nothing to style.
      if (text.asString().length > 0) {
        text.getTextStyle().setBold(true).setFontSize(titleSize)
          .setForegroundColor(theme.titleColor);
      }
    }
    if (s.bullets.length > 0) {
      const text = slide.insertTextBox('', margin, bodyTop, contentWidth,
        bodyHeight).getText();
      for (let i = 0; i < s.bullets.length; i++) {
        const b = s.bullets[i];
        // The first paragraph already exists in a fresh box; each later line appends one.  A
        // new paragraph starts at the previous line's bullet depth, so the level is set every
        // time.
        const para = i === 0 ? text : text.appendParagraph('').getRange();
        para.setBulletLevel(b.level);
        appendRuns(text, b.runs);
      }
      // Bullet lines whose runs were all empty leave the box without text, and then there is
      // nothing to style.
      if (text.asString().length > 0) {
        text.getTextStyle().setFontSize(bodySize).setForegroundColor(theme.bodyColor);
      }
    }
    built++;
  }
  return { built: built, total: p.getSlides().length };
};
