// Hello World in JavaScript
// Import standard OpenOffice.org API classes. For more information on
// these classes and the OpenOffice.org API, see the OpenOffice.org
// Developers Guide at:
// http://api.openoffice.org/

importClass(Packages.com.sun.star.uno.UnoRuntime);
importClass(Packages.com.sun.star.text.XTextDocument);
importClass(Packages.com.sun.star.text.XText);
importClass(Packages.com.sun.star.text.XTextRange);
importClass(Packages.com.sun.star.frame.XModel);

// Import XScriptContext class. An instance of this class is available
// to all JavaScript scripts in the global variable "XSCRIPTCONTEXT". This
// variable can be used to access the document for which this script
// was invoked.
// 
// Methods available are: 
// 
//   XSCRIPTCONTEXT.getDocument() returns XModel
//   XSCRIPTCONTEXT.getInvocationContext() returns XScriptInvocationContext or NULL
//   XSCRIPTCONTEXT.getDesktop() returns XDesktop
//   XSCRIPTCONTEXT.getComponentContext() returns XComponentContext
//
// For more information on using this class see the scripting
// developer guides at:
// 
//   http://api.openoffice.org/docs/DevelopersGuide/ScriptingFramework/ScriptingFramework.xhtml
//

oDoc = UnoRuntime.queryInterface(XModel,XSCRIPTCONTEXT.getInvocationContext());
if ( !oDoc )
  oDoc = XSCRIPTCONTEXT.getDocument();
xTextDoc = UnoRuntime.queryInterface(XTextDocument,oDoc);
xText = xTextDoc.getText();
xTextRange = xText.getEnd();
xTextRange.setString( "Hello World (in JavaScript)" );
