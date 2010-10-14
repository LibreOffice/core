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

#ifndef LAYOUT_CORE_IMPORT_HXX
#define LAYOUT_CORE_IMPORT_HXX

#include <map>
#include <list>
#define _BACKWARD_BACKWARD_WARNING_H 1
#include <hash_map>


#include <com/sun/star/xml/input/XRoot.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>

namespace layoutimpl
{
class LayoutRoot;
class LayoutWidget;
namespace css = ::com::sun::star;

class RadioGroups
{
public:
    RadioGroups();

    void addItem( rtl::OUString id, css::uno::Reference< css::awt::XRadioButton > xRadio )
        throw (css::uno::RuntimeException);

private:
    class RadioGroup : public ::cppu::WeakImplHelper1< css::awt::XItemListener >
        , public ::cppu::WeakImplHelper1< css::awt::XActionListener >
    {
    public:
        RadioGroup();
        void addItem( css::uno::Reference< css::awt::XRadioButton > xRadio );

    private:
        typedef std::list< css::uno::Reference< css::awt::XRadioButton > > RadioButtonsList;
        RadioButtonsList mxRadios;
        css::uno::Reference< css::awt::XRadioButton > mxSelectedRadio;

        void handleSelected ()
            throw (css::uno::RuntimeException);

        // awt::XItemListener
        void SAL_CALL itemStateChanged( const css::awt::ItemEvent& e )
            throw (css::uno::RuntimeException);

        // awt::XActionListener
        void SAL_CALL actionPerformed( const css::awt::ActionEvent& e )
            throw (css::uno::RuntimeException);

        // lang::XEventListener
        void SAL_CALL disposing( const css::lang::EventObject& )
            throw (css::uno::RuntimeException);
    };

    // each RadioGroup will stay alive after RadioGroups die with the ImportContext
    // because they are referenced by every XRadioButton through the listener
    typedef std::map< rtl::OUString, css::uno::Reference< RadioGroup > > RadioGroupsMap;
    RadioGroupsMap mxRadioGroups;
};

#if 0
// generator
class Widget
{
public:
    Widget( css::uno::Reference< css::awt::XToolkit > xToolkit,
            css::uno::Reference< css::awt::XWindow > xToplevel,
            rtl::OUString unoName, long attrbs );
    virtual ~Widget();

    virtual void setProperties( const PropList &rProps );

    virtual bool addChild( Widget *pChild );
    virtual void setChildProperties( Widget *pChild, const PropList &rProps );

    inline css::uno::Reference< css::awt::XLayoutConstrains > getPeer()
    { return mxWidget; }

    inline css::uno::Reference< css::awt::XLayoutConstrains > getContainer()
    { return mxContainer; }

protected:
    css::uno::Reference< css::awt::XLayoutConstrains > mxWidget;
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;
};

class Root
{
public:
    Root( css::uno::Reference< css::awt::XToolkit > xToolkit )
        : mxToolkit( xToolkit ) {}
    ~Root();

    virtual Widget *create( rtl::OUString id, const rtl::OUString unoName, long attrbs );

    css::uno::Reference< css::awt::XLayoutConstrains > getById( rtl::OUString id );
    inline css::uno::Reference< css::awt::XLayoutConstrains > getToplevel();

protected:
    css::uno::Reference< css::awt::XToolkit > mxToolkit;
    Widget *mpToplevel;

    typedef std::hash_map< rtl::OUString, css::uno::Reference< css::awt::XLayoutConstrains >,
                           rtl::OUStringHash > ItemHash;
    ItemHash maItems;
};
#endif

// parser
class ImportContext : public ::cppu::WeakImplHelper1< css::xml::input::XRoot >
{
public:
    sal_Int32 XMLNS_LAYOUT_UID, XMLNS_CONTAINER_UID;
    LayoutRoot &mrRoot; // switch to XNameContainer ref ?
    RadioGroups mxRadioGroups;

    inline ImportContext( LayoutRoot &rRoot ) SAL_THROW( () )
        : mrRoot( rRoot ) {}
    virtual ~ImportContext() {}

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping >
        const & xNamespaceMapping )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & /* rTarget */, ::rtl::OUString const & /* rData */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & /* xLocator */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startRootElement(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes > const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

class ElementBase : public ::cppu::WeakImplHelper1< css::xml::input::XElement >
{
protected:
    ImportContext *mpImport;
/* TODO: check if all this memebers are needed. */
    ElementBase   *mpParent;
    sal_Int32      mnUid;

    ::rtl::OUString maLocalName;
    css::uno::Reference< css::xml::input::XAttributes > mxAttributes;
public:
    ElementBase(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, ImportContext * pImport )
    SAL_THROW( () );
    virtual ~ElementBase() SAL_THROW(());

    // XElement
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL getParent()
        throw (css::uno::RuntimeException)
    { return static_cast< css::xml::input::XElement * >( mpParent ); }
    virtual ::rtl::OUString SAL_CALL getLocalName() throw (css::uno::RuntimeException)
    { return maLocalName; }
    virtual sal_Int32 SAL_CALL getUid() throw (css::uno::RuntimeException)
    { return mnUid; }
    virtual css::uno::Reference< css::xml::input::XAttributes >
    SAL_CALL getAttributes() throw (css::uno::RuntimeException)
    { return mxAttributes; }

    virtual void SAL_CALL ignorableWhitespace(
        ::rtl::OUString const & /* rWhitespaces */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL characters( ::rtl::OUString const & /* rChars */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & /* Target */, ::rtl::OUString const & /* Data */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }

    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes > const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException) = 0;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
};

class WidgetElement : public ElementBase
{
protected:
    LayoutWidget *mpWidget;

public:
    WidgetElement( sal_Int32 nUid, rtl::OUString const &name,
                   css::uno::Reference< css::xml::input::XAttributes > const &attributes,
                   ElementBase *parent, ImportContext *import ) SAL_THROW (());

    ~WidgetElement();


    virtual css::uno::Reference< css::xml::input::XElement> SAL_CALL
    startChildElement (sal_Int32 id, rtl::OUString const &name,
                       css::uno::Reference< css::xml::input::XAttributes > const &attributes)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException );
    virtual void SAL_CALL characters( ::rtl::OUString const & /* rChars */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

class ToplevelElement : public WidgetElement
{
public:
    ToplevelElement( rtl::OUString const &name,
                     css::uno::Reference< css::xml::input::XAttributes > const &attributes,
                     ImportContext *import ) SAL_THROW (());
    ~ToplevelElement();
};


} // namespace layoutimpl

#endif /* LAYOUT_CORE_IMPORT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
