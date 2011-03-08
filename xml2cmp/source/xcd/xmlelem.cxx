/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <xmlelem.hxx>


// NOT FULLY DEFINED SERVICES
#include <parse.hxx>
#include <cr_html.hxx>

XmlElement::XmlElement( const char * i_sName )
    : sName(i_sName)
{
}

void
XmlElement::Insert2Index( Index & ) const
{
    //  Default version. Does nothing.
}

XmlElement *
MultipleElement::FindChild( const Simstr & i_sChildName )
{
    unsigned i_max = aChildren.size();
    for ( unsigned i = 0; i < i_max; ++i )
    {
        if ( aChildren[i]->Name() == i_sChildName )
            return aChildren[i];
    }

    return 0;
}

MultipleElement::~MultipleElement()
{
}

MultipleElement::MultipleElement( const char * i_sName )
    :   XmlElement(i_sName)
{
}

void
MultipleElement::AddChild( XmlElement & let_drElement )
{
    aChildren.push_back(&let_drElement);
}

void
SequenceElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_Sequence( Children(), Name() );
}

void
SequenceElement::Write2Html( HtmlCreator & io_rHC ) const
{
    io_rHC.StartTable();

    Children()[nIndexNameElement]->Write2Html(io_rHC);
    for ( unsigned i = 0; i < Children().size(); ++i )
    {
        if (i != nIndexNameElement)
        {
            Children()[i]->Write2Html(io_rHC);
        }
    }  // end for

    io_rHC.FinishTable();
}


SequenceElement::SequenceElement( const char * i_sName,
                                  unsigned     i_nIndexNameElement )
    :   MultipleElement(i_sName),
        nIndexNameElement(i_nIndexNameElement)
{
}

FreeChoiceElement::FreeChoiceElement()
    :   MultipleElement("")
{
}

void
FreeChoiceElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_FreeChoice(Children());
}

void
FreeChoiceElement::Write2Html( HtmlCreator & io_rHC ) const
{
    for ( unsigned i = 0; i < Children().size(); ++i )
    {
        Children()[i]->Write2Html(io_rHC);
    }  // end for
}

ListElement::ListElement( const char * i_sElementsName,
                          F_CREATE     i_fCreateNewElement )
    :   MultipleElement(i_sElementsName),
        fCreateNewElement(i_fCreateNewElement)
{
}

void
ListElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_List(  *this );
}

void
ListElement::Write2Html( HtmlCreator & io_rHC ) const
{
    for ( unsigned i = 0; i < Children().size(); ++i )
    {
        Children()[i]->Write2Html(io_rHC);
    }  // end for
}

XmlElement *
ListElement::Create_and_Add_NewElement()
{
    XmlElement * pNew = (*fCreateNewElement)(Name());
    Children().push_back( pNew );
    return pNew;
}

TextElement::TextElement( const char *      i_sName,
                          E_LinkType        i_eLinkType,
                          bool              i_bReverseName )
    :   XmlElement(i_sName),
        eLinkType(i_eLinkType),
        bReverseName(i_bReverseName)
{
}

SglTextElement::SglTextElement( const char *        i_sName,
                                E_LinkType          i_eLinkType,
                                bool                i_bReverseName )
    :   TextElement(i_sName, i_eLinkType, i_bReverseName)
{
}

void
SglTextElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_Text(sContent, Name(), IsReversedName());
}

void
SglTextElement::Write2Html( HtmlCreator & io_rHC ) const
{
    if ( !sContent.is_no_text() )
        io_rHC.Write_SglTextElement( *this );
}

MultipleTextElement::MultipleTextElement( const char *      i_sName,
                                          E_LinkType        i_eLinkType,
                                          bool              i_bReverseName )
    :   TextElement(i_sName, i_eLinkType, i_bReverseName)
{
}

void
MultipleTextElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_MultipleText(aContent, Name(), IsReversedName());
}

void
MultipleTextElement::Write2Html(    HtmlCreator & io_rHC ) const
{
    if (Size() > 0)
        io_rHC.Write_MultiTextElement( *this );
}

const Simstr &
MultipleTextElement::Data( unsigned i_nNr ) const
{
    static const Simstr sNull_;

    if (i_nNr < aContent.size())
        return aContent[i_nNr];
    return sNull_;
}

EmptyElement::EmptyElement( const char * i_sName )
    :   XmlElement(i_sName)
{
}

SglAttrElement::SglAttrElement( const char *        i_sName,
                                const char *        i_sAttrName )
    :   EmptyElement(i_sName),
        sAttrName(i_sAttrName)
{
}

void
SglAttrElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_SglAttr(sAttrValue, Name(), sAttrName);
}

void
SglAttrElement::Write2Html( HtmlCreator & io_rHC ) const
{
    io_rHC.Write_SglText( Name(), sAttrValue );
}

MultipleAttrElement::MultipleAttrElement( const char *      i_sName,
                                          const char **     i_sAttrNames,
                                          unsigned          i_nSize )
    :   EmptyElement(i_sName)
{
    for ( unsigned i = 0; i < i_nSize; ++i )
    {
        aAttrNames.push_back(Simstr(i_sAttrNames[i]));
        aAttrValues.push_back(Simstr(""));
    }
}

void
MultipleAttrElement::Parse( X2CParser & io_rParser )
{
    io_rParser.Parse_MultipleAttr(aAttrValues, Name(), aAttrNames);
}

void
MultipleAttrElement::Write2Html(    HtmlCreator & io_rHC ) const
{
    if ( ! aAttrValues[0].is_no_text() )
        io_rHC.Write_ReferenceDocu( Name(), aAttrValues[0], aAttrValues[1], aAttrValues[2] );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
