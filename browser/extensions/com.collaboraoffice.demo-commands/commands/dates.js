var commands = {
	insertDate: function () {
		cool.getActiveDocument().getCursor().insertText(new Date().toLocaleDateString());
	},
	insertTime: function () {
		cool.getActiveDocument().getCursor().insertText(new Date().toLocaleTimeString());
	},
	insertIsoDate: function () {
		cool.getActiveDocument().getCursor().insertText(new Date().toISOString().slice(0, 10));
	},
	insertLocaleDate: function () {
		cool.getActiveDocument().getCursor().insertText(new Date().toLocaleDateString(undefined, {
			weekday: 'long',
			year: 'numeric',
			month: 'long',
			day: 'numeric',
		}));
	},
};
