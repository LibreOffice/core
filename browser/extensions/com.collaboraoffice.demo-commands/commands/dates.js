// Insert text the way typing does, replacing the selection when there is one:
function insertText(text) {
	var doc = cool.getActiveDocument();
	var selection = doc.getSelection();
	if (selection) selection.replace(text);
	else doc.getCursor().insertText(text);
}

var commands = {
	insertDate: function () {
		insertText(new Date().toLocaleDateString());
	},
	insertTime: function () {
		insertText(new Date().toLocaleTimeString());
	},
	insertIsoDate: function () {
		insertText(new Date().toISOString().slice(0, 10));
	},
	insertLocaleDate: function () {
		insertText(new Date().toLocaleDateString(undefined, {
			weekday: 'long',
			year: 'numeric',
			month: 'long',
			day: 'numeric',
		}));
	},
};
