var commands = {
	heading1: function () {
		cool.getActiveDocument().getCursor().uno.setPropertyValue('ParaStyleName', 'Heading 1');
	},
	heading2: function () {
		cool.getActiveDocument().getCursor().uno.setPropertyValue('ParaStyleName', 'Heading 2');
	},
	heading3: function () {
		cool.getActiveDocument().getCursor().uno.setPropertyValue('ParaStyleName', 'Heading 3');
	},
};
