'''
  This file is part of the LibreOffice project.
 
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 
  This file incorporates work covered by the following license notice:
 
    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''
import unittest
import unohelper
import os
from com.sun.star.lang import XMultiServiceFactory
from com.sun.star.text import XTextDocument
from com.sun.star.text import XTextField
from com.sun.star.container import XEnumeration
from com.sun.star.util import XRefreshable
from com.sun.star.container import XEnumerationAccess
from com.sun.star.beans import XPropertySet
from com.sun.star.text import XTextFieldsSupplier
from com.sun.star.container import XNamed 
from com.sun.star.text.ReferenceFieldPart import NUMBER
from com.sun.star.text.ReferenceFieldPart import NUMBER_NO_CONTEXT
from com.sun.star.text.ReferenceFieldPart import NUMBER_FULL_CONTEXT
from com.sun.star.text.ReferenceFieldSource import BOOKMARK
from com.sun.star.text.ReferenceFieldPart import TEXT

class CheckCrossReferences(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        document = util.WriterTools.loadTextDoc(UnoRuntime.queryInterface(type(XMultiServiceFactory).__name__, connection.getComponentContext().getServiceManager()), TestDocument.getUrl("CheckCrossReferences.odt"))

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()


    def getNextField():
        while True:
            while xPortionEnum == None:
                if (not(xParaEnum.hasMoreElements())):
                    self.fail("Cannot retrieve next field.")

                aPara = UnoRuntime.queryInterface(type(XEnumerationAccess).__name__, xParaEnum.nextElement())

                xPortionEnum = aPara.createEnumeration()
          
            if ( xPortionEnum == None ):
                break

            while xPortionEnum.hasMoreElements():
                xPortionProps = UnoRuntime.queryInterface(type(XPropertySet).__name__, xPortionEnum.nextElement())
                sPortionType = str(xPortionProps.getPropertyValue( "TextPortionType" ))
                if ( sPortionType.equals( "TextField") ):
                    xField = UnoRuntime.queryInterface(type(XTextField).__name__, xPortionProps.getPropertyValue( "TextField" ))
                    self.assertTrue("Cannot retrieve next field.", xField)
                    return xField
          
            xPortionEnum = None
        return None 
 
    def getFieldProps(xField ):
        Props = UnoRuntime.queryInterface(type(XPropertySet).__name__, xField )
        self.assertTrue("Cannot retrieve field properties.", xProps)
        return xProps
 
    def checkField(xField , xProps, nFormat, aExpectedFieldResult ):
        # set requested format
        xProps.setPropertyValue("ReferenceFieldPart", int(nFormat))

        # refresh fields in order to get new format applied
        xFieldsRefresh.refresh()
        aFieldResult = xField.getPresentation(False)
        self.assertEqual( "set reference field format doesn't result in correct field result",aExpectedFieldResult, aFieldResult)

    def checkCrossReferences():
        xParaEnumAccess = UnoRuntime.queryInterface(type(XEnumerationAccess).__name__, document.getText())
        xParaEnum = xParaEnumAccess.createEnumeration();

        #get field refresher
        xFieldSupp = UnoRuntime.queryInterface(type(XTextFieldsSupplier).__name__, document)
        xFieldsRefresh = UnoRuntime.queryInterface(type(XRefreshable).__name__, xFieldSupp.getTextFields())

        #check first reference field
        #strings for checking
        FieldResult1 = "*i*"
        FieldResult2 = "+b+*i*"
        FieldResult3 = "-1-+b+*i*"
        FieldResult4 = "1"
        FieldResult5 = "1"
        FieldResult6 = "A.1"
        FieldResult7 = "2(a)"
        FieldResult8 = "2(b)"
        FieldResult9 = "2"
        FieldResult10 = "1(a)"
        FieldResult11 = "(b)"
        FieldResult12 = "(a)" 

        #variables for current field
        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult2 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult1 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult3 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult1 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult1 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult3 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult3 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult1 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult3 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult5 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult4 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult6 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult4 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult4 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult6 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult6 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult4 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult6 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult7 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult12 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult7 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult8 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult11 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult8 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult9 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult9 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult9 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult4 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult4 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult4 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult10 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult12 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult10 )

        xField = getNextField()
        xProps = getFieldProps(xField)
        checkField( xField, xProps, NUMBER, FieldResult12 )
        checkField( xField, xProps, NUMBER_NO_CONTEXT, FieldResult12 )
        checkField( xField, xProps, NUMBER_FULL_CONTEXT, FieldResult7 )
        
        #insert a certain cross-reference bookmark and a reference field to this bookmark
        # restart paragraph enumeration
        xParaEnumAccess = UnoRuntime.queryInterface(type(XEnumerationAccess).__name__, document.getText())
        xParaEnum = xParaEnumAccess.createEnumeration()

        # iterate on the paragraphs to find certain paragraph to insert the bookmark
        while xParaEnum.hasMoreElements():
            xParaTextRange = UnoRuntime.queryInterface(type(XTextRange).__name__, xParaEnum.nextElement())

            if xParaTextRangexParaTextRange== "J" :
                break
            else:
                xParaTextRange = None
            self.assertTrue("Cannot find paragraph to insert cross-reference bookmark.", xParaTextRange)

            #insert bookmark
            xFac =UnoRuntime.queryInterface(type(XMultiServiceFactory).__name__, document)
            cBookmarkName = "__RefNumPara__47114711"
            xBookmark = UnoRuntime.queryInterface(type(XTextContent).__name__, xFac.createInstance( "com.sun.star.text.Bookmark" ))
            if xBookmark != None:
                xName =  UnoRuntime.queryInterface(type(XNamed).__name, xBookmark )
                xName.setName( cBookmarkName )
                xBookmark.attach(xParaTextRange.getStart())

            # insert reference field, which references the inserted bookmark
            xNewField =UnoRuntime.queryInterface(type(XTextContent).__name__,xFac.createInstance( "com.sun.star.text.TextField.GetReference" ))
            if xNewField != None:
                xFieldProps = UnoRuntime.queryInterface(type(XPropertySet).__name__, xNewField )
                xFieldProps.setPropertyValue( "ReferenceFieldPart", int(TEXT))
                xFieldProps.setPropertyValue( "ReferenceFieldSource", int(BOOKMARK))
                xFieldProps.setPropertyValue( "SourceName", cBookmarkName )
                xFieldTextRange =UnoRuntime.queryInterface(type(XTextRange).__name__, xParaEnum.nextElement())
                xNewField.attach(xFieldTextRange.getEnd())
                xFieldsRefresh.refresh()
                
            #check inserted reference field
            xField =UnoRuntime.queryInterface( type(XTextField).__name__, xNewField )
            self.assertEqual( "inserted reference field doesn't has correct field result", "J", xField.getPresentation(False))

            xParaTextRange.getStart().setString( "Hallo new bookmark: ")
            xFieldsRefresh.refresh()
            self.assertEqual("inserted reference field doesn't has correct field result", "Hallo new bookmark: J", xField.getPresentation(	False))





