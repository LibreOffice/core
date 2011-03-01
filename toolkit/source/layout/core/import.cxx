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

#include "import.hxx"

#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <vcl/image.hxx>
#include <tools/debug.hxx>
#include <layout/layout.hxx>

#include "root.hxx"
#include "helper.hxx"
#include "dialogbuttonhbox.hxx"


#define XMLNS_LAYOUT_URI    "http://openoffice.org/2007/layout"
#define XMLNS_CONTAINER_URI "http://openoffice.org/2007/layout/container"

namespace layoutimpl
{
using namespace css;

using ::rtl::OUString;

ElementBase::~ElementBase()
SAL_THROW( () )
{
    //delete mpImport;
    //mpImport = 0;
}

//** parser
WidgetElement::WidgetElement ( sal_Int32 nUid, const OUString &rName,
                               uno::Reference <xml::input::XAttributes> const &attributes,
                               ElementBase *pParent,
                               ImportContext *pImport)
SAL_THROW (())
: ElementBase( nUid, rName, attributes, pParent, pImport )
{
    OUString name = rName.toAsciiLowerCase();

    PropList aProps;
    propsFromAttributes( attributes, aProps, pImport->XMLNS_LAYOUT_UID );

    OUString aId;
    findAndRemove( "id", aProps, aId );
    OUString aLang;
    findAndRemove( "xml-lang", aProps, aLang );

    {
//DEBUG
        uno::Reference< awt::XLayoutConstrains > xParent;
        if ( pParent )
            xParent = ((WidgetElement *) pParent)->mpWidget->getPeer();


        mpWidget = pImport->mrRoot.create( aId, name,
                                           getAttributeProps( aProps ), uno::Reference< awt::XLayoutContainer >( xParent, uno::UNO_QUERY ) );

    }

    // TODO: handle with non-existing widgets

    mpWidget->setProperties( aProps );

    uno::Reference< awt::XDialog2 > xDialog( mpWidget->getPeer(), uno::UNO_QUERY );
    if ( xDialog.is() )
    {
        OUString aTitle;
        if ( findAndRemove( "title", aProps, aTitle ) )
        {
            OSL_TRACE("Setting title: %s", OUSTRING_CSTR( aTitle ) );
            xDialog->setTitle( aTitle );
        }
        OUString aHelpId;
        if ( findAndRemove( "help-id", aProps, aHelpId ) )
        {
            OSL_TRACE("Setting help-id: %s", OUSTRING_CSTR( aHelpId ) );
            xDialog->setHelpId( aHelpId.toInt32 () );
        }
    } // DEBUG:
    else if ( pParent == NULL )
    {
        OSL_FAIL( "Fatal error: top node isn't a dialog" );
    }

    OUString aOrdering;
    if ( findAndRemove( "ordering", aProps, aOrdering ) )
        if ( DialogButtonHBox *b = dynamic_cast<DialogButtonHBox *> ( mpWidget->getPeer().get() ) )
            b->setOrdering ( aOrdering );

    bool bSetRadioGroup;
    OUString aRadioGroup;
    bSetRadioGroup = findAndRemove( "radiogroup", aProps, aRadioGroup );

    mpWidget->setProperties( aProps );

    // we need to add radio buttons to the group after their properties are
    // set, so we can check if they should be the one selected by default or not.
    // And the state changed event isn't fired when changing properties.

    uno::Reference< awt::XRadioButton > xRadio( mpWidget->getPeer(), uno::UNO_QUERY );
    if ( xRadio.is() )
    {
        if (!bSetRadioGroup)
            aRadioGroup = OUString(RTL_CONSTASCII_USTRINGPARAM ("default"));
        pImport->mxRadioGroups.addItem( aRadioGroup, xRadio );
    }
}

WidgetElement::~WidgetElement()
{
    //delete mpWidget;
    //mpWidget = 0;
}

uno::Reference <xml::input::XElement>
WidgetElement::startChildElement ( sal_Int32 nUid, OUString const &name,
                                   uno::Reference <xml::input::XAttributes> const &attributes )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    // Adding a child to the widget
    WidgetElement *pChild = new WidgetElement ( nUid, name, attributes, this, mpImport );

    if ( !mpWidget->addChild( pChild->mpWidget ) )
    {
        DBG_ERROR2( "ERROR: cannot add %s to container %s, container full", OUSTRING_CSTR( name ), OUSTRING_CSTR( getLocalName() ) );
        throw xml::sax::SAXException();
    }

    PropList aProps;
    propsFromAttributes( attributes, aProps, mpImport->XMLNS_CONTAINER_UID );
    mpWidget->setChildProperties( pChild->mpWidget, aProps );

    return pChild;
}

// Support Ivo Hinkelmann's move label/text/title attribute to CONTENT
// transex3 hack.
void SAL_CALL
WidgetElement::characters( OUString const& rChars )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    if ( mpWidget && rChars.trim().getLength() )
    {
        uno::Reference< awt::XDialog2 > xDialog( mpWidget->getPeer(), uno::UNO_QUERY );
        uno::Reference< awt::XButton > xButton( mpWidget->getPeer(), uno::UNO_QUERY );
        if ( xDialog.is() )
            xDialog->setTitle( rChars );
        else if ( xButton.is() )
            mpWidget->setProperty( OUString(RTL_CONSTASCII_USTRINGPARAM("label")), rChars );
        else
            mpWidget->setProperty( OUString(RTL_CONSTASCII_USTRINGPARAM("text")), rChars );
    }
}
// ---- ElementBase ----

ElementBase::ElementBase( sal_Int32 nUid, OUString const & rLocalName,
                          uno::Reference< xml::input::XAttributes > const & xAttributes,
                          ElementBase* pParent,
                          ImportContext* pImport )
SAL_THROW(())
: mpImport( pImport )
    , mpParent( pParent )
    , mnUid( nUid )
    , maLocalName( rLocalName )
    , mxAttributes( xAttributes )
{
}

// ---- ImportContext ----

void ImportContext::startDocument(
    uno::Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    XMLNS_LAYOUT_UID = xNamespaceMapping->getUidByUri(
        OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_LAYOUT_URI ) ) );
    XMLNS_CONTAINER_UID = xNamespaceMapping->getUidByUri(
        OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_CONTAINER_URI ) ) );
}

ToplevelElement::ToplevelElement (OUString const &rName,
                                  uno::Reference <xml::input::XAttributes> const &xAttributes,
                                  ImportContext *pImport)
SAL_THROW(())
: WidgetElement( 0, rName, xAttributes, NULL, pImport )
{
}

ToplevelElement::~ToplevelElement()
{
}

uno::Reference< xml::input::XElement > ImportContext::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    uno::Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    if ( XMLNS_LAYOUT_UID != nUid )
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "invalid namespace!" ) ),
            uno::Reference< uno::XInterface >(), uno::Any() );
        return new ToplevelElement( rLocalName, xAttributes, this );
}

RadioGroups::RadioGroups()
{
}

void RadioGroups::addItem( rtl::OUString id, uno::Reference< awt::XRadioButton > xRadio )
    throw (uno::RuntimeException)
{
    if ( ! xRadio.is() )
        throw uno::RuntimeException();

    uno::Reference< RadioGroup > group;
    RadioGroupsMap::iterator it = mxRadioGroups.find( id );
    if ( it == mxRadioGroups.end() )
    {
        group = uno::Reference< RadioGroup > ( new RadioGroup() );
        mxRadioGroups [id] = group;
    }
    else
        group = it->second;
    group->addItem( xRadio );
}

RadioGroups::RadioGroup::RadioGroup()
{
}

void RadioGroups::RadioGroup::addItem( uno::Reference< awt::XRadioButton > xRadio )
{
    if ( ! mxSelectedRadio.is() )
    {
        xRadio->setState( true );
        mxSelectedRadio = xRadio;
    }
    else if ( xRadio->getState() )
    {
#if 1
        xRadio->setState( false );
#else // huh, why select last added?
      mxSelectedRadio->setState( false );
      mxSelectedRadio = xRadio;
#endif
    }

    // TOO late: actionPerformed is called before itemStateChanged.
    // If client code (wrongly?) uses actionPerformed, it will see
    // the previous RadioButtons' state.
    xRadio->addItemListener( this );

    uno::Reference< awt::XButton > xButton = uno::Reference< awt::XButton > ( xRadio, uno::UNO_QUERY );
    xButton->addActionListener( this );

    mxRadios.push_back (xRadio);
}

void RadioGroups::RadioGroup::handleSelected ()
    throw (uno::RuntimeException)
{
    for ( RadioButtonsList::iterator it = mxRadios.begin();
          it != mxRadios.end(); ++it )
        if ( *it != mxSelectedRadio && (*it)->getState() )
        {
            mxSelectedRadio->setState( false );
            mxSelectedRadio = *it;
            break;
        }
}

// awt::XItemListener
void RadioGroups::RadioGroup::itemStateChanged( const awt::ItemEvent& e )
    throw (uno::RuntimeException)
{
    // TOO late: actionPerformed is called before itemStateChanged.
    // If client code (wrongly?) uses actionPerformed, it will see
    // the previous RadioButtons' state.

    // Need this for initialization, though.
    if ( e.Selected )
        handleSelected ();
}

// awt::XActionListener
void RadioGroups::RadioGroup::actionPerformed( const awt::ActionEvent& )
    throw (uno::RuntimeException)
{
    handleSelected ();
}

// lang::XEventListener
void SAL_CALL RadioGroups::RadioGroup::disposing( const lang::EventObject& )
    throw (uno::RuntimeException)
{
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
