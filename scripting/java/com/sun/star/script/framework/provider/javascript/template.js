/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
// Hello World in JavaScript
// Import standard OpenOffice.org API classes. For more information on
// these classes and the OpenOffice.org API, see the OpenOffice.org
// Developers Guide at:
// http://api.libreoffice.org/

importClass(Packages.com.sun.star.uno.UnoRuntime);
importClass(Packages.com.sun.star.text.XTextDocument);
importClass(Packages.com.sun.star.text.XText);
importClass(Packages.com.sun.star.text.XTextRange);
importClass(Packages.com.sun.star.frame.XModel);

// Import XScriptContext class. An instance of this class is available
// to all JavaScript scripts in the global variable "XSCRIPTCONTEXT". This
// variable can be used to access the document for which this script
// was invoked.

// Methods available are:

//   XSCRIPTCONTEXT.getDocument() returns XModel
//   XSCRIPTCONTEXT.getInvocationContext() returns XScriptInvocationContext or NULL
//   XSCRIPTCONTEXT.getDesktop() returns XDesktop
//   XSCRIPTCONTEXT.getComponentContext() returns XComponentContext

// For more information on using this class see the scripting
// developer guides at:

//   http://api.libreoffice.org/docs/DevelopersGuide/ScriptingFramework/ScriptingFramework.xhtml


oDoc = UnoRuntime.queryInterface(XModel,XSCRIPTCONTEXT.getInvocationContext());
if ( !oDoc )
  oDoc = XSCRIPTCONTEXT.getDocument();
xTextDoc = UnoRuntime.queryInterface(XTextDocument,oDoc);
xText = xTextDoc.getText();
xTextRange = xText.getEnd();
xTextRange.setString( "Hello World (in JavaScript)" );
