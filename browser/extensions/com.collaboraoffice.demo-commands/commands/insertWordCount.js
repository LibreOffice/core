var commands = {
	insertWordCount: function () {
		var doc = cool.getActiveDocument();
		var text = doc.getBody().getText().trim();
		var words = text.length ? text.split(/\s+/).length : 0;
		var selection = doc.getSelection();
		if (selection) selection.replace('Word count: ' + words);
		else doc.getCursor().insertText('Word count: ' + words);
	},
};
