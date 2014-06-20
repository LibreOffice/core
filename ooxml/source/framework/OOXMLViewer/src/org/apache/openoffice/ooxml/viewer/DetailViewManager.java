/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/

package org.apache.openoffice.ooxml.viewer;

import java.util.Vector;

import javax.swing.JScrollPane;

import org.apache.openoffice.ooxml.framework.part.ContentType;
import org.apache.openoffice.ooxml.framework.part.OOXMLPackage;
import org.apache.openoffice.ooxml.framework.part.Part;
import org.apache.openoffice.ooxml.framework.part.PartName;
import org.apache.openoffice.ooxml.framework.part.parser.ParserFactory;
import org.apache.openoffice.ooxml.viewer.tokenview.TokenView;
import org.apache.openoffice.ooxml.viewer.xml.TokenType;
import org.apache.openoffice.ooxml.viewer.xml.XMLTokenViewFactory;
import org.apache.openoffice.ooxml.viewer.xmltokenview.XMLViewFactory;

public class DetailViewManager
{
    public DetailViewManager (
        final JScrollPane aDetailViewContainer,
        final OOXMLPackage aPackage)
    {
        maDetailViewContainer = aDetailViewContainer;
        maPackage = aPackage;
        maViewFactory = new XMLTokenViewFactory();
    }




    public void ShowPart (
        final PartName aName,
        final ContentType eType)
    {
        ShowPart(maPackage.getPart(aName));
    }




    public void ShowPart (final Part aPart)
    {
        switch(aPart.getContentType())
        {
            case ImageGIF:
            case ImageJPG:
            case ImagePNG:
                maDetailViewContainer.setViewportView(
                    new ImageView(aPart.getStream()).GetComponent());
                break;

            case ApplicationDrawing:
            case ApplicationExcel:
            case ApplicationXML:
            case Chart:
            case ContentTypes:
            case CoreProperties:
            case CustomXMLProperties:
            case ExtendedProperties:
            case PmlDocument:
            case PmlHandoutMaster:
            case PmlNotesMaster:
            case PmlNotesSlide:
            case PmlProperties:
            case PmlSlide:
            case PmlSlideLayout:
            case PmlSlideMaster:
            case PmlTableStyles:
            case PmlViewProperties:
            case Relationships:
            case SmlSheet:
            case Theme:
            case ThemeOverride:
            case WmlDocument:
            case WmlEndNotes:
            case WmlFontTable:
            case WmlFootNotes:
            case WmlFooter:
            case WmlHeader:
            case WmlNumbering:
            case WmlSettings:
            case WmlStyles:
            case WmlWebSettings:
                final TokenView<TokenType> aTokenView = maViewFactory.Create(aPart.getStream());
                maDetailViewContainer.setViewportView(aTokenView);
                final Vector<String> aErrorsAndWarnings = new Vector<>();
                XMLViewFactory.AddSemanticInformation(
                    aTokenView,
                    ParserFactory.getParser(
                        aPart.getContentType(),
                        aPart.getStream(),
                        aErrorsAndWarnings),
                    aErrorsAndWarnings);
                break;

            case OleObject:
            case Unknown:
            default:
                maDetailViewContainer.setViewportView(null);
                break;
        }
    }




    private final JScrollPane maDetailViewContainer;
    private XMLTokenViewFactory maViewFactory;
    private OOXMLPackage maPackage;
}
