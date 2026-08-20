var commands = {
	heading1: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		viewCursor.setPropertyValue('ParaStyleName', 'Heading 1');
	},
	heading2: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		viewCursor.setPropertyValue('ParaStyleName', 'Heading 2');
	},
	heading3: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		viewCursor.setPropertyValue('ParaStyleName', 'Heading 3');
	},
};
