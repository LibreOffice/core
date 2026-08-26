var commands = {
	insertWordCount: function () {
		var doc = cool.getActiveDocument();
		var text = doc.getBody().getText().trim();
		var words = text.length ? text.split(/\s+/).length : 0;
		doc.getCursor().insertText('Word count: ' + words);
	},
};
