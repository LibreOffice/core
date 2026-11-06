remote.apply = function apply(greeting, colorize) {
  console.log('script called with:', greeting, colorize);
  const doc = DocumentApp.getActiveDocument();
  doc.getBody().getChild(0).editAsText().insertText(0, greeting);
  if (colorize) {
    for (const para of doc.getBody().getChildren()) {
      const color = '#' + Math.floor(Math.random() * 0x1000000).toString(16).padStart(6, '0');
      try {
        para.editAsText().setForegroundColor(color);
      } catch (e) {
        console.log('caught:', e);
      }
    }
  }
};

remote.colorizeParagraphs = function colorizeParagraphs(color) {
  for (const para of DocumentApp.getActiveDocument().getBody().getChildren()) {
    try {
      para.editAsText().setForegroundColor(color);
    } catch (e) {
      console.log('caught:', e);
    }
  }
};

remote.readParagraph = function readParagraph(n) {
  return DocumentApp.getActiveDocument().getBody().getChildren()[n - 1].getText();
};
