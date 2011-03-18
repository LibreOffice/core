// Hello World in JavaScript
importClass(Packages.com.sun.star.uno.UnoRuntime);
importClass(Packages.com.sun.star.text.XTextDocument);
importClass(Packages.com.sun.star.text.XText);
importClass(Packages.com.sun.star.text.XTextRange);

//get the document from the scripting context
oDoc = XSCRIPTCONTEXT.getDocument();
//get the XTextDocument interface
xTextDoc = UnoRuntime.queryInterface(XTextDocument,oDoc);
//get the XText interface
xText = xTextDoc.getText();
//get an (empty) XTextRange interface at the end of the text
xTextRange = xText.getEnd();
//set the text in the XTextRange
xTextRange.setString( "Hello World (in JavaScript)" );
