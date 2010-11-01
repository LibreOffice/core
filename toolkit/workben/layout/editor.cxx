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

#include "editor.hxx"

#undef NDEBUG

/*
#include <stdio.h>
#include <string.h>
*/

#include <cassert>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/property.hxx>
#include <vcl/lstbox.h>

using namespace layout::css;

using rtl::OUString;

// FIXME:
//#define FILEDLG

#include <layout/core/helper.hxx>
#include <layout/core/root.hxx>
#include <layout/core/helper.hxx>

// TODO: automatically generated
struct WidgetSpec {
    const char *pLabel, *pName, *pIconName;
    bool bIsContainer; };
static const WidgetSpec WIDGETS_SPECS[] = {
    { "Label",         "fixedtext"   , "sc_label.png",        false },
    { "Button",        "pushbutton"  , "sc_pushbutton.png",   false },
    { "Radio Button",  "radiobutton" , "sc_radiobutton.png",  false },
    { "Check Box",     "checkbox"    , "sc_checkbox.png",     false },
    { "Line Edit",     "edit"        , "sc_edit.png",         false },
    { "Numeric Field", "numericfield", "sc_numericfield.png", false },
    { "List Box                  ", "listbox"     , NULL,                  false },
    // containers
    { "Hor Box",       "hbox"        , NULL,                  true  },
    { "Ver Box",       "vbox"        , NULL,                  true  },
    { "Table",         "table"       , NULL,                  true  },
    { "Alignment",     "align"       , NULL,                  true  },
    { "Tab Control",   "tabcontrol"  , NULL,                  true  },
    { "Hor Splitter",  "hsplitter"   , NULL,                  true  },
    { "Ver Splitter",  "vsplitter"   , NULL,                  true  },
    { "Scroller",      "scroller"    , NULL,                  true  },
};
const int WIDGETS_SPECS_LEN = sizeof (WIDGETS_SPECS) / sizeof (WidgetSpec);

using namespace layout;
using namespace layoutimpl;
namespace css = ::com::sun::star;

static rtl::OUString anyToString (uno::Any value)
{
    try
    {
        switch (value.getValueTypeClass()) {
            case uno::TypeClass_STRING:
                return value.get<rtl::OUString>();
            case uno::TypeClass_CONSTANT:
                return rtl::OUString::valueOf (value.get<sal_Int32>());
            case uno::TypeClass_LONG:
                return rtl::OUString::valueOf (value.get<sal_Int64>());
            case uno::TypeClass_SHORT:
                // FIXME: seems broken
                return rtl::OUString::valueOf ((sal_Int32) value.get<short>());

            case uno::TypeClass_FLOAT:
                return rtl::OUString::valueOf (value.get<float>());
            case uno::TypeClass_DOUBLE:
                return rtl::OUString::valueOf (value.get<double>());

            case uno::TypeClass_BOOLEAN:
            {
                bool val = value.get<sal_Bool>();
                return rtl::OUString( val ? "1" : "0", 1, RTL_TEXTENCODING_ASCII_US );
/*                if ( val )
                  return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) );
                  else
                  return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) );*/
            }
            default:
                break;
        }
    }
    catch(...) {}
    return rtl::OUString();
}

static inline long anyToNatural (uno::Any value)
{ return sal::static_int_cast<long>(anyToString( value ).toInt64()); }
static inline double anyToDecimal (uno::Any value)
{ return anyToString( value ).toDouble(); }

/* XLayoutContainer/XLayoutConstrains are a bit of a hasle to work with.
   Let's wrap them. */
class Widget : public layoutimpl::LayoutWidget
{
    friend class EditorRoot;

    Widget *mpParent;
    std::vector< Widget *> maChildren;
    bool mbForeign;

    rtl::OUString mrId;
    rtl::OUString mrLabel, mrUnoName;

// TODO: store original properties. And some property handling methods.
    long mnOriAttrbs;
    layoutimpl::PropList maOriProps, maOriChildProps;

public:

    // to be used to wrap the root
    Widget( uno::Reference< awt::XLayoutConstrains > xImport, const char *label )
        : mpParent( 0 ), mbForeign( true )
    {
        mxWidget = xImport;
        mxContainer = uno::Reference< awt::XLayoutContainer >( mxWidget, uno::UNO_QUERY );

        mrLabel = rtl::OUString( label, strlen( label ), RTL_TEXTENCODING_UTF8  );
    }

    Widget( rtl::OUString id, uno::Reference< awt::XToolkit > xToolkit,
            uno::Reference< awt::XLayoutContainer > xParent,
            rtl::OUString unoName, long nAttrbs )
        : mpParent( 0 ), mbForeign( false ), mrId( id ),
          mnOriAttrbs( nAttrbs )
    {
        while ( xParent.is() && !uno::Reference< awt::XWindow >( xParent, uno::UNO_QUERY ).is() )
        {
            uno::Reference< awt::XLayoutContainer > xContainer( xParent, uno::UNO_QUERY );
            OSL_ASSERT( xContainer.is() );
            xParent = uno::Reference< awt::XLayoutContainer >( xContainer->getParent(), uno::UNO_QUERY );
        }

        mxWidget = WidgetFactory::createWidget( xToolkit, xParent, unoName, nAttrbs );
        OSL_ASSERT( mxWidget.is() );
        mxContainer = uno::Reference< awt::XLayoutContainer >( mxWidget, uno::UNO_QUERY );

        mrLabel = mrUnoName = unoName;
        // try to get a nicer label for the widget
        for ( int i = 0; i < WIDGETS_SPECS_LEN; i++ )
            if ( unoName.equalsAscii( WIDGETS_SPECS[ i ].pName ) )
            {
                const char *label = WIDGETS_SPECS[ i ].pLabel;
                mrLabel = rtl::OUString( label, strlen( label ), RTL_TEXTENCODING_UTF8  );
                break;
            }

        // set default Text property
        // TODO: disable editing of text fields, check boxes selected, etc...


        // store original properties
        {
            PropertyIterator it( this, WINDOW_PROPERTY );
            while ( it.hasNext() )
            {
                beans::Property prop = it.next();
                rtl::OUString name( prop.Name );
                rtl::OUString value( getProperty( name, WINDOW_PROPERTY ) );
#if DEBUG_PRINT
                fprintf(stderr, "original property: %s = %s\n", OUSTRING_CSTR(name), OUSTRING_CSTR(value));
#endif
                std::pair< rtl::OUString, rtl::OUString > pair( name, value );
                maOriProps.push_back( pair );
            }
        }

    }

    ~Widget()
    {
        for ( std::vector< Widget *>::const_iterator it = maChildren.begin();
             it != maChildren.end(); it++ )
            delete *it;
        if ( !mbForeign )
        {
            uno::Reference< lang::XComponent > xComp( mxWidget, uno::UNO_QUERY );
            if ( xComp.is() )
                // some widgets, like our containers, don't implement this interface...
                xComp->dispose();
        }
    }

    uno::Reference< awt::XLayoutConstrains > impl()
    {
        return mxWidget;
    }

    // LayoutWidget
    virtual bool addChild( LayoutWidget *pChild )
    {
        return addChild( static_cast< Widget * >( pChild ) );
    }

    virtual void setProperties( const PropList &rProps )
    {
//        maOriProps = rProps;
        LayoutWidget::setProperties( rProps );
    }

    virtual void setChildProperties( LayoutWidget *pChild, const PropList &rProps )
    {
        maOriChildProps = rProps;
        LayoutWidget::setChildProperties( pChild, rProps );
    }

    // tree travel
    Widget *up()
    {
        return mpParent;
    }

    Widget *down()
    {
        if ( maChildren.empty() )
            return NULL;
        return maChildren.front();
    }

    Widget *next()
    {
        if ( mpParent )
        {
            int pos = mpParent->getChildPos( this );
            return mpParent->getChild( pos+1 );
        }
        return NULL;
    }

    Widget *prev()
    {
        if ( mpParent )
        {
            int pos = mpParent->getChildPos( this );
            return mpParent->getChild( pos-1 );
        }
        return NULL;
    }

    // handle
    bool addChild( Widget *pChild, int pos = 0xffff )
    {
        if ( !mxContainer.is() )
            return false;

        uno::Sequence< uno::Reference < awt::XLayoutConstrains > > aChildren;
        aChildren = mxContainer->getChildren();
        int nChildrenLen = aChildren.getLength();

        // ugly, but let's check if the container is next to full...
        try {
            mxContainer->addChild( pChild->mxWidget );
        }
        catch( awt::MaxChildrenException ex ) {
            return false;
        }

        if ( pos < nChildrenLen )
        {  // if its on the middle, we need to make space for it
            mxContainer->removeChild( pChild->mxWidget );
            for ( int i = pos; i < nChildrenLen; i++ )
                mxContainer->removeChild( aChildren[ i ] );
            mxContainer->addChild( pChild->mxWidget );
            for ( int i = pos; i < nChildrenLen; i++ )
                mxContainer->addChild( aChildren[ i ] );
            maChildren.insert( maChildren.begin()+pos, pChild );
        }
        else
            maChildren.push_back( pChild );

        OSL_ASSERT( pChild->mpParent == NULL );
        pChild->mpParent = this;

        // store container props
        {
            pChild->maOriChildProps.clear();
            PropertyIterator it( pChild, CONTAINER_PROPERTY );
            while ( it.hasNext() )
            {
                beans::Property prop = it.next();
                rtl::OUString name( prop.Name );
        try {
            rtl::OUString value( pChild->getProperty( name, CONTAINER_PROPERTY ) );
            std::pair< rtl::OUString, rtl::OUString > pair( name, value );
            pChild->maOriChildProps.push_back( pair );
        } catch ( beans::UnknownPropertyException &rEx ) {
            fprintf (stderr, "ERROR: widget reports that it has a property it cannot return: '%s' this normally means that someone screwed up their PROPERTY_SET_INFO macro usage.\n",
                 rtl::OUStringToOString (rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
            }
        }

        return true;
    }

    bool removeChild( Widget *pChild )
    {
        if ( !mxContainer.is() || pChild->mpParent != this )
            return false;

        mxContainer->removeChild( pChild->mxWidget );

        unsigned int pos = getChildPos( pChild );
        if ( pos < maChildren.size() )
            maChildren.erase( maChildren.begin()+pos );
        pChild->mpParent = NULL;

        return true;
    }

    bool swapWithChild( Widget *pChild )
    {
        if ( !pChild->isContainer() )
            return false;

        // remove all child's childrens, and try to add them here
        removeChild( pChild );

        // keep a copy for failure
        std::vector< Widget *> aChildren = maChildren;
        std::vector< Widget *> aChildChildren = pChild->maChildren;

        for ( std::vector< Widget *>::const_iterator it = aChildChildren.begin();
              it != aChildChildren.end(); it++ )
            pChild->removeChild( *it );

        for ( std::vector< Widget *>::const_iterator it = aChildChildren.begin();
              it != aChildChildren.end(); it++ )
            if ( !addChild( *it ) )
            {    // failure
                for ( std::vector< Widget *>::const_iterator jt = aChildChildren.begin();
                      jt != it; jt++ )
                    removeChild( *jt );
                for ( std::vector< Widget *>::const_iterator jt = aChildChildren.begin();
                      jt != aChildChildren.end(); jt++ )
                    pChild->addChild( *jt );
                return false;
            }

        Widget *pParent = up();

        if ( pParent )
        {
            pParent->removeChild( this );
            pParent->addChild( pChild );
        }
        pChild->addChild( this );
        return true;
    }

    unsigned int getChildPos( Widget *pChild )
    {
        int i = 0;
        for ( std::vector< Widget *>::const_iterator it = maChildren.begin();
              it != maChildren.end(); it++, i++ )
            if ( *it == pChild )
                break;
        return i;
    }

    Widget *getChild( int pos )
    {
        if ( pos >= 0 && pos < (signed) maChildren.size() )
            return *(maChildren.begin() + pos);
        return NULL;
    }

    bool isContainer()
    { return mxContainer.is(); }
    unsigned int getChildrenLen()
    { return maChildren.size(); }

    rtl::OUString getLabel() const
    { return mrLabel; }
    rtl::OUString getUnoName() const
    { return mrUnoName; }

    int getDepth()
    {
        int depth = 0;
        for ( Widget *pWidget = mpParent; pWidget; pWidget = pWidget->mpParent )
            depth++;
        return depth;
    }

    enum PropertyKind {
        WINDOW_PROPERTY, CONTAINER_PROPERTY, WINBITS_PROPERTY
    };

    static rtl::OUString findProperty( const PropList &props, rtl::OUString propName )
    {
        for ( PropList::const_iterator it = props.begin(); it != props.end(); it++ )
            if ( it->first.equalsIgnoreAsciiCase( propName ) )
                return it->second;
#if DEBUG_PRINT
        fprintf(stderr, "Serious error: property '%s' not found\n", OUSTRING_CSTR(propName));
#endif
        return rtl::OUString();
    }

    rtl::OUString getOriginalProperty( rtl::OUString rPropName, PropertyKind rKind )
    {
        rtl::OUString rValue;
        switch ( rKind ) {
            case WINDOW_PROPERTY:
                rValue = findProperty( maOriProps, rPropName );
                break;
            case CONTAINER_PROPERTY:
                rValue = findProperty( maOriChildProps, rPropName );
                break;
            case WINBITS_PROPERTY:
                // TODO
                break;
        }

        return rValue;
    }

    rtl::OUString getProperty( rtl::OUString rPropName, PropertyKind rKind )
    {
        rtl::OUString rValue;
        switch ( rKind ) {
            case WINDOW_PROPERTY:
                rValue = anyToString( layoutimpl::prophlp::getProperty( mxWidget, rPropName ) );
                break;
            case CONTAINER_PROPERTY:
                if ( mpParent )
                    rValue = anyToString( layoutimpl::prophlp::getProperty(
                                              mpParent->mxContainer->getChildProperties( mxWidget ), rPropName ) );
                break;
            case WINBITS_PROPERTY:
                // TODO
                break;
        }

        return rValue;
    }

    bool isPropertyTouched( rtl::OUString propName, PropertyKind rKind )
    {
        rtl::OUString oriValue = getOriginalProperty( propName, rKind );
        rtl::OUString newValue = getProperty( propName, rKind );
        bool isTouched = oriValue != newValue;
#if DEBUG_PRINT
        fprintf(stderr, "is property '%s' touched? %s  (%s vs %s)\n", OUSTRING_CSTR(propName), isTouched ? "yes" : "no", OUSTRING_CSTR(oriValue), OUSTRING_CSTR(newValue));
#endif
        return isTouched;
    }

    using LayoutWidget::setProperty;

    void setProperty( rtl::OUString rPropName, PropertyKind rKind, uno::Any rValue )
    {
        switch ( rKind ) {
            case WINDOW_PROPERTY:
                layoutimpl::prophlp::setProperty( mxWidget, rPropName, rValue );
                break;
            case CONTAINER_PROPERTY:
                if ( mpParent )
                    layoutimpl::prophlp::setProperty(
                        mpParent->mxContainer->getChildProperties( mxWidget ), rPropName, rValue );
                break;
            case WINBITS_PROPERTY:
                // TODO
                break;
        }
    }

    struct PropertyIterator {
        friend class Widget;
        PropertyKind mrKind;
        uno::Sequence< beans::Property > maProps;
        int nPropIt;

        PropertyIterator( Widget *pWidget, PropertyKind rKind )
            : mrKind( rKind ), nPropIt( 0 )
        {
            switch ( rKind )
            {
                case WINDOW_PROPERTY:
                    if ( layoutimpl::prophlp::canHandleProps( pWidget->mxWidget ) )
                    {
                        uno::Reference< beans::XPropertySetInfo > xInfo
                            = layoutimpl::prophlp::queryPropertyInfo( pWidget->mxWidget );
                        if ( !xInfo.is() )
                            return;

                        maProps = xInfo->getProperties();
                    }
                    break;
                case CONTAINER_PROPERTY:
                    if ( pWidget->mpParent )
                    {
                        uno::Reference< beans::XPropertySet >xParentSet(
                            pWidget->mpParent->mxContainer->getChildProperties( pWidget->mxWidget ) );
                        if ( xParentSet.is())
                        {
                            uno::Reference< beans::XPropertySetInfo > xInfo( xParentSet->getPropertySetInfo() );
                            if ( xInfo.is() )
                                maProps = xInfo->getProperties();
                        }
                    }
                    break;
                case WINBITS_PROPERTY:
                    // TODO
                    break;
            }
        }

        bool hasNext()
        {
            return nPropIt < maProps.getLength();
        }

        beans::Property next()
        {
/*            rtl::OUString propName, propValue;
              propName = maProps[ nPropIt ];
              propValue = getProperty( propName, mrKind, false);
              nPropIt++;
              return std::pair< rtl::OUString, rtl::OUString > propPair( propName, propValue );*/
            return maProps[ nPropIt++ ];
        }
    };
};

class EditorRoot : public layoutimpl::LayoutRoot {
    Widget *mpParent;

public:
    EditorRoot( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                Widget *pParent )
        : layoutimpl::LayoutRoot( xFactory ), mpParent( pParent )
    {
    }

    // generation
    virtual layoutimpl::LayoutWidget *create( rtl::OUString id, const rtl::OUString unoName,
                                              long attrbs, uno::Reference< awt::XLayoutContainer > xParent )
    {
        if ( unoName.compareToAscii( "dialog" ) == 0 )
            return mpParent;

        // TODO: go through specs to map unoName to a more human-readable label
        Widget *pWidget = new Widget( id, mxToolkit, xParent, unoName, attrbs );
        if ( !mxWindow.is() )
            mxWindow = uno::Reference< awt::XWindow >( pWidget->getPeer(), uno::UNO_QUERY );

        if ( pWidget->mxContainer.is() )
            pWidget->mxContainer->setLayoutUnit( mpParent->mxContainer->getLayoutUnit() );

        return pWidget;
    }
};

/* Working with the layout in 1D, as if it was a flat list. */
namespace FlatLayout
{
Widget *next( Widget *pWidget )
{
    Widget *pNext;
    pNext = pWidget->down();
    if ( pNext ) return pNext;
    pNext = pWidget->next();
    if ( pNext ) return pNext;
    for ( Widget *pUp = pWidget->up(); pUp != NULL; pUp = pUp->up() )
        if ( (pNext = pUp->next()) != NULL )
            return pNext;
    return NULL;
}

/*
  Widget *prev( Widget *pWidget )
  {
  Widget *pPrev;
  pPrev = pWidget->prev();
  if ( !pPrev )
  return pWidget->up();

  Widget *pBottom = pPrev->down();
  if ( pBottom )
  {
  while ( pBottom->down() || pBottom->next() )
  {
  for ( Widget *pNext = pBottom->next(); pNext; pNext = pNext->next() )
  pBottom = pNext;
  Widget *pDown = pBottom->down();
  if ( pDown )
  pBottom = pDown;
  }
  return pBottom;
  }
  return pPrev;
  }
*/

bool moveWidget( Widget *pWidget, bool up /*or down*/ )
{
    // Keep child parent&pos for in case of failure
    Widget *pOriContainer = pWidget->up();
    unsigned int oriChildPos = pOriContainer->getChildPos( pWidget );

    // Get parent&sibling before removing it, since relations get cut
    Widget *pSibling = up ? pWidget->prev() : pWidget->next();
    Widget *pContainer = pWidget->up();
    if ( !pContainer )
        return false;

    // try to swap with parent or child
    // We need to allow for this at least for the root node...
    if ( !pSibling )
    {
        if ( up )
        {
            if ( pContainer->swapWithChild( pWidget ) )
                return true;
        }
        else
        {
        }
    }

    pContainer->removeChild( pWidget );

    // if has up sibling -- append to it, else swap with it
    if ( pSibling )
    {
        if ( pSibling->addChild( pWidget, up ? 0xffff : 0 ) )
            return true;

        unsigned int childPos = pContainer->getChildPos( pSibling );
        if ( pContainer->addChild( pWidget, childPos + (up ? 0 : 1) ) )
            return true;  // should always be succesful
    }
    // go through parents -- try to get prepended to them
    else
    {
        for ( ; pContainer && pContainer->up(); pContainer = pContainer->up() )
        {
            unsigned int childPos = pContainer->up()->getChildPos( pContainer );
            if ( pContainer->up()->addChild( pWidget, childPos + (up ? 0 : 1) ) )
                return true;
        }
    }

    // failed -- try to get it to its old position
    if ( !pOriContainer->addChild( pWidget, oriChildPos ) )
    {
        // a parent should never reject a child back. but if it ever
        // happens, just kill it, we don't run an orphanate here ;P
        delete pWidget;
        return true;
    }
    return false;
}

// NOTE: root is considered to be number -1
Widget *get( Widget *pRoot, int nb )
{
    Widget *it;
    for ( it = pRoot; it != NULL && nb >= 0; it = next( it ) )
        nb--;
    return it;
}

int get( Widget *pRoot, Widget *pWidget )
{
    int nRet = -1;
    Widget *it;
    for ( it = pRoot; it != NULL && it != pWidget; it = next( it ) )
        nRet++;
    return nRet;
}
}

//** PropertiesList widget

class PropertiesList : public layout::Table
{
    class PropertyEntry
    {
        friend class PropertiesList;

        /* wrapper between the widget and Any */
        struct AnyWidget
        {
            DECL_LINK( ApplyPropertyHdl, layout::Window* );
            DECL_LINK( FlagToggledHdl, layout::CheckBox* );

            AnyWidget( Widget *pWidget, rtl::OUString aPropName, Widget::PropertyKind aPropKind )
                : mpWidget( pWidget ), maPropName( aPropName ), maPropKind( aPropKind )
            {
                mpFlag = 0;
                mbBlockFlagCallback = false;
                bFirstGet = true;
            }

            virtual ~AnyWidget()
            {
#if DEBUG_PRINT
                fprintf(stderr, "~AnyWidget\n");
#endif
            }

            void save( uno::Any aValue )
            {
                mpWidget->setProperty( maPropName, maPropKind, aValue );
                checkProperty();
            }

            void checkProperty()
            {
                bool flag = mpWidget->isPropertyTouched( maPropName, maPropKind );

                if ( mpFlag && mpFlag->IsChecked() != (BOOL)flag )
                {
                    CheckFlag( flag, true );
                }
            }

            void CheckFlag( bool bValue, bool bBlockCallback )
            {
                if ( bBlockCallback )
                    mbBlockFlagCallback = true;
                mpFlag->Check( bValue );
                mbBlockFlagCallback = false;
            }

            bool bFirstGet;  // HACK
            rtl::OUString getValue()
            {
//                return mpWidget->getOriProperty( maPropName );
                rtl::OUString value;
                if ( bFirstGet )    // king of ugliness
                    value = mpWidget->getProperty( maPropName, maPropKind );
                else
                    value = mpWidget->getOriginalProperty( maPropName, maPropKind );
                bFirstGet = false;
                return value;
            }

            // FIXME: wrapper should have a base class for this...
            virtual layout::Window *getWindow() = 0;
            virtual layout::Container *getContainer() { return NULL; }

            virtual void load() = 0;
            virtual void store() = 0;

            Widget *mpWidget;
            rtl::OUString maPropName;
            Widget::PropertyKind maPropKind;
            layout::CheckBox *mpFlag;
            bool mbBlockFlagCallback;
        };

        struct AnyEdit : public AnyWidget, layout::HBox
        {
            layout::Edit *mpEdit;
            bool mbMultiLine;
            layout::PushButton *mpExpand;
            DECL_LINK( ExpandEditHdl, layout::PushButton* );

            // so we can create widgets (like transforming the Edit into a
            // MultiLineEdit)
            layout::Window *mpWinParent;

            AnyEdit( Widget *pWidget, rtl::OUString aPropName,
                     Widget::PropertyKind aPropKind, layout::Window *pWinParent )
                : AnyWidget( pWidget, aPropName, aPropKind ), layout::HBox( 0, false ), mpWinParent( pWinParent )
            {
                mpEdit = NULL;
                mpExpand = new layout::PushButton( pWinParent, WB_TOGGLE );
                mpExpand->SetToggleHdl( LINK( this, AnyEdit, ExpandEditHdl ) );
                setAsMultiLine( false );

                load();
            }

            virtual ~AnyEdit()
            {
                delete mpEdit;
                delete mpExpand;
            }

            virtual layout::Window *getWindow()
            { return NULL; }
            virtual layout::Container *getContainer()
            { return this; }

            void setAsMultiLine( bool bMultiLine )
            {
                Clear();
                XubString text;
                if ( mpEdit )
                {
                    text = mpEdit->GetText();
                    printf("Remove mpEdit and expand\n");
                    Remove( mpEdit );
                    Remove( mpExpand );
                    delete mpEdit;
                }

                if ( bMultiLine )
                {
                    mpEdit = new layout::Edit( mpWinParent, WB_BORDER );
                    mpExpand->SetText( String::CreateFromAscii( "-" ) );
                }
                else
                {
                    mpEdit = new layout::Edit( mpWinParent, WB_BORDER );
                    mpExpand->SetText( String::CreateFromAscii( "+" ) );
                }

                mpEdit->SetText( text );
                mpEdit->SetModifyHdl( LINK( this, AnyEdit, ApplyPropertyHdl ) );

                Add( mpEdit, true, true, 0 );
                Add( mpExpand, false, true, 0 );

                mbMultiLine = bMultiLine;
            }

            virtual void load()
            {
                mpEdit->SetText( getValue() );
                checkProperty();
            }

            virtual void store()
            {
                save( uno::makeAny( (rtl::OUString) mpEdit->GetText() ) );
            }
        };

        struct AnyInteger : public AnyWidget, NumericField
        {
            AnyInteger( Widget *pWidget, rtl::OUString aPropName,
                        Widget::PropertyKind aPropKind, Window *pWinParent )
                : AnyWidget( pWidget, aPropName, aPropKind ), NumericField( pWinParent, WB_SPIN|WB_BORDER )
            {
                load();
                SetModifyHdl( LINK( this, AnyInteger, ApplyPropertyHdl ) );
            }

            virtual Window *getWindow()
            { return this; }

            virtual void load()
            {
                OUString text = getValue();
                SetText( text.getStr() );
                checkProperty();
            }

            virtual void store()
            {
#if DEBUG_PRINT
                fprintf(stderr, "store number: %ld\n", rtl::OUString( GetText() ).toInt64());
#endif
                save( uno::makeAny( rtl::OUString( GetText() ).toInt64() ) );
            }
        };

        struct AnyFloat : public AnyInteger
        {
            AnyFloat( Widget *pWidget, rtl::OUString aPropName,
                      Widget::PropertyKind aPropKind, Window *pWinParent )
                : AnyInteger( pWidget, aPropName, aPropKind, pWinParent )
            {}

            virtual void store()
            {
                save( uno::makeAny( rtl::OUString( GetText() ).toDouble() ) );
            }
        };

        struct AnyCheckBox : public AnyWidget, layout::CheckBox
        {
            AnyCheckBox( Widget *pWidget, rtl::OUString aPropName,
                         Widget::PropertyKind aPropKind, layout::Window *pWinParent )
                : AnyWidget( pWidget, aPropName, aPropKind ), layout::CheckBox( pWinParent )
            {
                // adding some whitespaces to make the hit area larger
//                SetText( String::CreateFromAscii( "" ) );
                load();
                SetToggleHdl( LINK( this, AnyWidget, ApplyPropertyHdl ) );
            }

            virtual ~AnyCheckBox()
            {
#if DEBUG_PRINT
                fprintf(stderr, "~AnyCheckBox\n");
#endif
            }

            virtual layout::Window *getWindow()
            { return this; }

            virtual void load()
            {
#if DEBUG_PRINT
                fprintf(stderr, "loading boolean value\n");
#endif
                Check( getValue().toInt64() != 0 );
                setLabel();
                checkProperty();
            }

            virtual void store()
            {
                save( uno::makeAny( IsChecked() ) );
                setLabel();
            }

            void setLabel()
            {
                SetText( String::CreateFromAscii( IsChecked() ? "true" : "false" ) );
            }
        };

        struct AnyListBox : public AnyWidget, layout::ListBox
        {
            AnyListBox( Widget *pWidget, rtl::OUString aPropName,
                        Widget::PropertyKind aPropKind, Window *pWinParent )
                : AnyWidget( pWidget, aPropName, aPropKind ), layout::ListBox( pWinParent, WB_DROPDOWN )
            {
                SetSelectHdl( LINK( this, AnyWidget, ApplyPropertyHdl ) );
            }

            virtual layout::Window *getWindow()
            { return this; }

            virtual void load()
            {
                SelectEntryPos( sal::static_int_cast< USHORT >( getValue().toInt32() ) );
                checkProperty();
            }

            virtual void store()
            {
                save( uno::makeAny( (short) GetSelectEntryPos() ) );
            }
        };

        struct AnyAlign : public AnyListBox
        {
            AnyAlign( Widget *pWidget, rtl::OUString aPropName,
                      Widget::PropertyKind aPropKind, Window *pWinParent )
                : AnyListBox( pWidget, aPropName, aPropKind, pWinParent )
            {
                InsertEntry( XubString::CreateFromAscii( "Left" ) );
                InsertEntry( XubString::CreateFromAscii( "Center" ) );
                InsertEntry( XubString::CreateFromAscii( "Right" ) );
                load();
            }
        };

        /* AnyListBox and AnyComboBox different in that a ComboBox allows the user
           to add other options, operating in strings, instead of constants.
           (its more like a suggestive AnyEdit) */
        struct AnyComboBox : public AnyWidget, layout::ComboBox
        {
            AnyComboBox( Widget *pWidget, rtl::OUString aPropName,
                         Widget::PropertyKind aPropKind, Window *pWinParent )
                : AnyWidget( pWidget, aPropName, aPropKind ), layout::ComboBox( pWinParent, WB_DROPDOWN )
            {
                SetModifyHdl( LINK( this, AnyComboBox, ApplyPropertyHdl ) );
            }

            virtual layout::Window *getWindow()
            { return this; }

            virtual void load()
            {
                SetText( getValue() );
                checkProperty();
            }

            virtual void store()
            {
                save( uno::makeAny( (rtl::OUString) GetText() ) );
            }
        };

        struct AnyFontStyle : public AnyComboBox
        {
            AnyFontStyle( Widget *pWidget, rtl::OUString aPropName,
                          Widget::PropertyKind aPropKind, Window *pWinParent )
                : AnyComboBox( pWidget, aPropName, aPropKind, pWinParent )
            {
                InsertEntry( XubString::CreateFromAscii( "Bold" ) );
                InsertEntry( XubString::CreateFromAscii( "Italic" ) );
                InsertEntry( XubString::CreateFromAscii( "Bold Italic" ) );
                InsertEntry( XubString::CreateFromAscii( "Fett" ) );
                load();
            }
        };

        layout::FixedText *mpLabel;
        layout::CheckBox *mpFlag;
        AnyWidget *mpValue;

    public:
        PropertyEntry( layout::Window *pWinParent, AnyWidget *pAnyWidget )
        {
            mpLabel = new layout::FixedText( pWinParent );
            {
                // append ':' to aPropName
                rtl::OUStringBuffer buf( pAnyWidget->maPropName );
                buf.append( sal_Unicode (':') );
                mpLabel->SetText( buf.makeStringAndClear() );
            }
            mpValue = pAnyWidget;
            mpFlag = new layout::CheckBox( pWinParent );
            mpFlag->SetToggleHdl( LINK( mpValue, AnyWidget, FlagToggledHdl ) );
            mpValue->mpFlag = mpFlag;
        }

        ~PropertyEntry()
        {
#if DEBUG_PRINT
                fprintf(stderr, "REMOVING label, flag and value\n");
#endif
            delete mpLabel;
            delete mpFlag;
            delete mpValue;
        }

        // Use this factory rather than the constructor -- check for NULL
        static PropertyEntry *construct( Widget *pWidget, rtl::OUString aPropName,
                                         Widget::PropertyKind aPropKind, sal_uInt16 nType,
                                         layout::Window *pWinParent )
        {
            AnyWidget *pAnyWidget;
            switch (nType) {
                case uno::TypeClass_STRING:
                    if ( aPropName.compareToAscii( "FontStyleName" ) == 0 )
                    {
                        pAnyWidget = new AnyFontStyle( pWidget, aPropName, aPropKind, pWinParent );
                        break;
                    }
                    pAnyWidget = new AnyEdit( pWidget, aPropName, aPropKind, pWinParent );
                    break;
                case uno::TypeClass_SHORT:
                    if ( aPropName.compareToAscii( "Align" ) == 0 )
                    {
                        pAnyWidget = new AnyAlign( pWidget, aPropName, aPropKind, pWinParent );
                        break;
                    }
                    // otherwise, treat as any other number...
                case uno::TypeClass_LONG:
                case uno::TypeClass_UNSIGNED_LONG:
                    pAnyWidget = new AnyInteger( pWidget, aPropName, aPropKind, pWinParent );
                    break;
                case uno::TypeClass_FLOAT:
                case uno::TypeClass_DOUBLE:
                    pAnyWidget = new AnyFloat( pWidget, aPropName, aPropKind, pWinParent );
                    break;
                case uno::TypeClass_BOOLEAN:
                    pAnyWidget = new AnyCheckBox( pWidget, aPropName, aPropKind, pWinParent );
                    break;
                default:
                    return NULL;
            }
            return new PropertyEntry( pWinParent, pAnyWidget );
        }
    };

    layout::Window *mpParentWindow;

    std::list< PropertyEntry* > maPropertiesList;
    layout::FixedLine *mpSeparator;

    // some properties are obscure, or simply don't make sense in this
    // context. Let's just ignore them.
    // Maybe we could offer them in an expander or something...
    static bool toIgnore( rtl::OUString prop )
    {
        // binary search -- keep the list sorted alphabetically
        static char const *toIgnoreList[] = {
            "DefaultControl", "FocusOnClick", "FontCharWidth", "FontCharset",
            "FontEmphasisMark", "FontFamily", "FontHeight", "FontKerning", "FontName",
            "FontOrientation", "FontPitch", "FontRelief", "FontSlant", "FontStrikeout",
            "FontType", "FontWordLineMode", "HelpText", "HelpURL", "MultiLine",
            "Printable", "Repeat", "RepeatDelay", "Tabstop"
        };

        int min = 0, max = sizeof( toIgnoreList )/sizeof( char * ) - 1, mid, cmp;
        do {
            mid = min + (max - min)/2;
            cmp = prop.compareToAscii( toIgnoreList[ mid ] );
            if ( cmp > 0 )
                min = mid+1;
            else if ( cmp < 0 )
                max = mid-1;
            else
                return true;
        } while ( min <= max );
        return false;
    }

public:
    PropertiesList( layout::Dialog *dialog )
        : layout::Table( dialog, "properties-box" )
        , mpParentWindow( dialog ), mpSeparator( 0 )
    {
    }

    ~PropertiesList()
    {
        clear();
    }

private:
    // auxiliary, add properties from the peer to the list
    void addProperties( Widget *pWidget, Widget::PropertyKind rKind )
    {
        Widget::PropertyIterator it( pWidget, rKind );
        while ( it.hasNext() )
        {
            beans::Property prop = it.next();
            rtl::OUString name( prop.Name );
            if ( toIgnore( name ) )
                continue;
            sal_uInt16 type = static_cast< sal_uInt16 >( prop.Type.getTypeClass() );

            PropertyEntry *propEntry = PropertyEntry::construct(
                pWidget, name, rKind, type, mpParentWindow );

            if ( propEntry )
            {
                Add( propEntry->mpLabel, false, false );

                // HACK: one of these will return Null...
                Add( propEntry->mpValue->getWindow(), true, false );
                Add( propEntry->mpValue->getContainer(), true, false );

                Add( propEntry->mpFlag, false, false );
                maPropertiesList.push_back( propEntry );
            }
        }
    }

public:
    void selectedWidget( Widget *pWidget )
    {
        clear();

        if ( !pWidget )
            return;

        addProperties( pWidget, Widget::CONTAINER_PROPERTY );

        mpSeparator = new layout::FixedLine( mpParentWindow );
        // TODO: we may want to have to separate list widgets here...
        Add( mpSeparator, false, false, 3, 1 );

        addProperties( pWidget, Widget::WINDOW_PROPERTY );

        ShowAll( true );
    }

    void clear()
    {
        ///FIXME: crash
        Container::Clear();

        for ( std::list< PropertyEntry* >::iterator it = maPropertiesList.begin();
              it != maPropertiesList.end(); it++)
            delete *it;
        maPropertiesList.clear();

        delete mpSeparator;
        mpSeparator = NULL;
    }
};

IMPL_LINK( PropertiesList::PropertyEntry::AnyWidget, ApplyPropertyHdl, layout::Window *, pWin )
{
    (void) pWin;
    store();
    return 0;
}

IMPL_LINK( PropertiesList::PropertyEntry::AnyWidget, FlagToggledHdl, layout::CheckBox *, pCheck )
{
#if DEBUG_PRINT
    fprintf(stderr, "Property flag pressed -- is: %d\n", pCheck->IsChecked());
#endif
    if ( !mbBlockFlagCallback )
    {
        bool checked = pCheck->IsChecked();
        if ( !checked )  // revert
        {
#if DEBUG_PRINT
            fprintf(stderr, "revert\n");
#endif
            load();
        }
        else
        {
#if DEBUG_PRINT
            fprintf(stderr, "user can't dirty the flag!\n");
#endif
            // User can't flag the property as dirty
            // Actually, we may want to allow the designer to force a property to be stored.
            // Could be useful when the default value of some new property wasn't yet decided...
            CheckFlag( false, true );
        }
    }
#if DEBUG_PRINT
    else
        fprintf(stderr, "Property flag pressed -- BLOCKED\n");
#endif
    return 0;
}

IMPL_LINK( PropertiesList::PropertyEntry::AnyEdit, ExpandEditHdl, layout::PushButton *, pBtn )
{
    setAsMultiLine( pBtn->IsChecked() );
    return 0;
}

//** SortListBox auxiliary widget

class SortListBox
{        // For a manual sort ListBox; asks for a ListBox and Up/Down/Remove
         // buttons to wrap
    DECL_LINK( ItemSelectedHdl, layout::ListBox* );
    DECL_LINK( UpPressedHdl, layout::Button* );
    DECL_LINK( DownPressedHdl, layout::Button* );
    DECL_LINK( RemovePressedHdl, layout::Button* );
    layout::PushButton *mpUpButton, *mpDownButton, *mpRemoveButton;

protected:
    layout::ListBox *mpListBox;

    virtual void upPressed( USHORT nPos )
    {
        XubString str = mpListBox->GetSelectEntry();
        mpListBox->RemoveEntry( nPos );
        nPos = mpListBox->InsertEntry( str, nPos-1 );
        mpListBox->SelectEntryPos( nPos );
    }

    virtual void downPressed( USHORT nPos )
    {
        XubString str = mpListBox->GetSelectEntry();
        mpListBox->RemoveEntry( nPos );
        nPos = mpListBox->InsertEntry( str, nPos+1 );
        mpListBox->SelectEntryPos( nPos );
    }

    virtual void removePressed( USHORT nPos )
    {
        mpListBox->RemoveEntry( nPos );
    }

    virtual void itemSelected( USHORT nPos )
    {
        // if we had some XLayoutContainer::canAdd() or maxChildren() function
        // we could make a function to check if we can move selected and enable/
        // /disable the move buttons as appropriate

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
        {
            mpUpButton->Disable();
            mpDownButton->Disable();
            mpRemoveButton->Disable();
        }
        else
        {
            mpUpButton->Enable();
            mpDownButton->Enable();
            mpRemoveButton->Enable();
        }
    }

public:
    SortListBox( layout::ListBox *pListBox, layout::PushButton *pUpButton, layout::PushButton *pDownButton,
                 layout::PushButton *pRemoveButton )
        : mpUpButton( pUpButton), mpDownButton( pDownButton), mpRemoveButton( pRemoveButton ),
          mpListBox( pListBox )
    {
        mpListBox->SetSelectHdl( LINK( this, SortListBox, ItemSelectedHdl ) );

        mpUpButton->SetModeImage( layout::Image ( "res/commandimagelist/lc_moveup.png" ) );
        mpUpButton->SetImageAlign( IMAGEALIGN_LEFT );
        mpUpButton->SetClickHdl( LINK( this, SortListBox, UpPressedHdl ) );

        mpDownButton->SetModeImage( layout::Image ( "res/commandimagelist/lc_movedown.png" ) );
        mpDownButton->SetImageAlign( IMAGEALIGN_LEFT );
        mpDownButton->SetClickHdl( LINK( this, SortListBox, DownPressedHdl ) );

        // "res/commandimagelist/lch_delete.png", "res/commandimagelist/lc_delete.png"
        mpRemoveButton->SetModeImage( layout::Image ( "res/commandimagelist/sc_closedoc.png" ) );
        mpRemoveButton->SetImageAlign( IMAGEALIGN_LEFT );
        mpRemoveButton->SetClickHdl( LINK( this, SortListBox, RemovePressedHdl ) );

        // fire an un-select event
        itemSelected( LISTBOX_ENTRY_NOTFOUND );
    }

    virtual ~SortListBox();
};

SortListBox::~SortListBox()
{
    delete mpListBox;
    delete mpUpButton;
    delete mpDownButton;
    delete mpRemoveButton;
}

IMPL_LINK( SortListBox, UpPressedHdl, layout::Button *, pBtn )
{
    (void) pBtn;
    USHORT pos = mpListBox->GetSelectEntryPos();
    if ( pos > 0 && pos != LISTBOX_ENTRY_NOTFOUND )
        upPressed( pos );
    return 0;
}

IMPL_LINK( SortListBox, DownPressedHdl, layout::Button *, pBtn )
{
    (void) pBtn;
    USHORT pos = mpListBox->GetSelectEntryPos();
    if ( pos < mpListBox->GetEntryCount() && pos != LISTBOX_ENTRY_NOTFOUND )
        downPressed( pos );
    return 0;
}

IMPL_LINK( SortListBox, RemovePressedHdl, layout::Button *, pBtn )
{
    (void) pBtn;
    USHORT pos = mpListBox->GetSelectEntryPos();
    if ( pos != LISTBOX_ENTRY_NOTFOUND )
        removePressed( pos );
    return 0;
}

IMPL_LINK( SortListBox, ItemSelectedHdl, layout::ListBox *, pList )
{
    (void) pList;
    USHORT pos = mpListBox->GetSelectEntryPos();
    itemSelected( pos );
    return 0;
}

//** LayoutTree widget

class LayoutTree : public SortListBox
{
public:
    struct Listener
    {
        virtual void widgetSelected( Widget *pWidget ) = 0;
    };

private:
    Listener *mpListener;

public:
    Widget *mpRootWidget;

    LayoutTree( layout::Dialog *dialog )
        : SortListBox( new layout::ListBox( dialog, "layout-tree" ),
                       new layout::PushButton( dialog, "layout-up-button" ),
                       new layout::PushButton( dialog, "layout-down-button" ),
                       new layout::PushButton( dialog, "layout-remove-button" ) )
    {
        layout::PeerHandle handle = dialog->GetPeerHandle( "preview-box" );
        uno::Reference< awt::XLayoutConstrains > xWidget( handle, uno::UNO_QUERY );
        mpRootWidget = new Widget( xWidget, "root" );
    }

    virtual ~LayoutTree();

    Widget *getWidget( int nPos )
    {
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            return FlatLayout::get( mpRootWidget, nPos );
        return NULL;
    }

    Widget *getSelectedWidget()
    {
        Widget *pWidget = getWidget( mpListBox->GetSelectEntryPos() );
        if ( !pWidget )  // return root, if none selected
            pWidget = mpRootWidget;
        return pWidget;
    }

    void selectWidget( Widget *pWidget )
    {
        int pos = FlatLayout::get( mpRootWidget, pWidget );
        if ( pos == -1 )
            // if asked to select hidden root, select visible root
            pos = 0;
        mpListBox->SelectEntryPos( sal::static_int_cast< USHORT >( pos ) );
    }

    void rebuild()
    {
        struct inner
        {
            // pads a string with whitespaces
            static rtl::OUString padString( rtl::OUString name, int depth )
            {
                rtl::OStringBuffer aBuf( depth * 4 + name.getLength() + 2 );
                for (int i = 0; i < depth; i++)
                    aBuf.append( "    " );
                aBuf.append( rtl::OUStringToOString( name, RTL_TEXTENCODING_ASCII_US ) );
                return rtl::OUString( aBuf.getStr(), aBuf.getLength(),
                                      RTL_TEXTENCODING_UTF8 );
            }
        };

        mpListBox->Clear();
        for ( Widget *i = FlatLayout::next( mpRootWidget ); i; i = FlatLayout::next( i ) )
            mpListBox->InsertEntry( inner::padString( i->getLabel(), i->getDepth()-1 ) );

        // any selection, no longer is. ListBox doesn't fire the event on this case;
        // force it.
        itemSelected( LISTBOX_ENTRY_NOTFOUND );
    }

    void setListener( Listener *pListener )
    { mpListener = pListener; }

    // print in XML format...

    static rtl::OUString toXMLNaming (const rtl::OUString &string)
    {
        rtl::OUStringBuffer buffer (string.getLength());
        sal_Unicode *str = string.pData->buffer;
        for (int i = 0; i < string.getLength(); i++) {
            if ( str[i] >= 'A' && str[i] <= 'Z' )
            {
                if ( i > 0 )
                    buffer.append ((sal_Unicode) '-');
                buffer.append ((sal_Unicode) (str[i] - 'A' + 'a'));
            }
            else
                buffer.append ((sal_Unicode) str[i]);
        }

        return buffer.makeStringAndClear();
    }

    void print()
    {
        printf("\t\tExport:\n");
        printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<dialog xmlns=\"http://openoffice.org/2007/layout\"\n"
               "        xmlns:cnt=\"http://openoffice.org/2007/layout/container\"\n"
               "        id=\"dialog\" title=\"Unnamed\" sizeable=\"true\" >\n");

        for ( Widget *i = FlatLayout::next( mpRootWidget ); i; i = FlatLayout::next( i ) )
        {
            for ( int d = i->getDepth(); d > 0; d-- )
                printf("    ");
            printf("<%s ", OUSTRING_CSTR( i->getUnoName() ) );

            for ( int kind = 0; kind < 2; kind++ )
            {
                Widget::PropertyKind wKind = kind == 0 ? Widget::WINDOW_PROPERTY
                    : Widget::CONTAINER_PROPERTY;
                Widget::PropertyIterator it( i, wKind );
                while ( it.hasNext() )
                {
                    beans::Property prop = it.next();
                    if ( !i->isPropertyTouched( prop.Name, wKind ) )
                        continue;

                    rtl::OUString value = i->getProperty( prop.Name, wKind );
                    if ( prop.Type.getTypeClass() == uno::TypeClass_BOOLEAN )
                    {
                        if ( value.compareToAscii( "0" ) )
                            value = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("false") );
                        else
                            value = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("true") );
                    }

                    if ( value.getLength() > 0 )
                        printf("%s%s=\"%s\" ",
                               kind == 0 ? "" : "cnt:",
                               OUSTRING_CSTR( toXMLNaming( prop.Name ) ), OUSTRING_CSTR( value )
                            );

                }
            }
            printf("/>\n");
        }
        printf("</dialog>\n");
    }

protected:
    virtual void upPressed( USHORT nPos )
    {
        Widget *pWidget = getWidget( nPos );
        if ( FlatLayout::moveWidget( pWidget, true ) )
            rebuild();
        selectWidget( pWidget );
    }

    virtual void downPressed( USHORT nPos )
    {
        Widget *pWidget = getWidget( nPos );
        if ( FlatLayout::moveWidget( pWidget, false ) )
            rebuild();
        selectWidget( pWidget );
    }

    virtual void removePressed( USHORT nPos )
    {
        Widget *pWidget = getWidget( nPos );
        if ( pWidget )
        {
            pWidget->up()->removeChild( pWidget );
            delete pWidget;
            rebuild();
        }
    }

    virtual void itemSelected( USHORT nPos )
    {
        mpListener->widgetSelected( getWidget( nPos ) );
        SortListBox::itemSelected( nPos );
    }
};

LayoutTree::~LayoutTree()
{
    delete mpRootWidget;
}

//** EditorImpl

class EditorImpl : public LayoutTree::Listener
{
    void createWidget( const char *unoName );

    PropertiesList *mpPropertiesList;
    LayoutTree *mpLayoutTree;

    layout::PushButton *pImportButton, *pExportButton;
#ifdef FILEDLG
    FileDialog *pImportDialog;
#endif
    DECL_LINK( ImportButtonHdl, layout::PushButton* );
    DECL_LINK( ExportButtonHdl, layout::PushButton* );
#ifdef FILEDLG
    DECL_LINK( ImportDialogHdl, FileDialog* );
#endif

    // framework stuff
    uno::Reference< lang::XMultiServiceFactory > mxFactory;
    uno::Reference< awt::XToolkit > mxToolkit;
    uno::Reference< awt::XWindow > mxToplevel;

    virtual void widgetSelected( Widget *pWidget );
    DECL_LINK( CreateWidgetHdl, layout::Button* );

    std::list< layout::PushButton *> maCreateButtons;

public:

    EditorImpl( layout::Dialog *dialog,
                // we should probable open this channel (or whatever its called) ourselves
                uno::Reference< lang::XMultiServiceFactory > xMSF );
    virtual ~EditorImpl();

    void loadFile( const rtl::OUString &aTestFile );
};

EditorImpl::EditorImpl( layout::Dialog *dialog,
                        uno::Reference< lang::XMultiServiceFactory > xFactory )
    : mxFactory( xFactory )
    , mxToplevel( dialog->GetPeerHandle( "dialog" ), uno::UNO_QUERY )
    // FIXME: any of these should work
    //dialog->getContext()->getRoot(), uno::UNO_QUERY )
    // dialog->GetPeer(), uno::UNO_QUERY )
{
#if DEBUG_PRINT
    fprintf (stderr, "EditorImpl()\n");
#endif
    // framework
    mxToolkit = uno::Reference< awt::XToolkit >(
        mxFactory->createInstance(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ) ) ),
        uno::UNO_QUERY );
    OSL_ASSERT( mxToolkit.is() );

    // custom widgets
#if DEBUG_PRINT
    fprintf (stderr, "custom widgets\n");
#endif
    mpPropertiesList = new PropertiesList( dialog );

    mpLayoutTree = new LayoutTree( dialog );
    mpLayoutTree->setListener( this );

/*    if ( xImport.is() )
      mpLayoutTree->getWidget( -1 )->addChild( new Widget( xImport, "import" ) );*/

    // create buttons
    layout::Container aWidgets( dialog, "create-widget" );
    layout::Container aContainers( dialog, "create-container" );
    for ( int i = 0; i < WIDGETS_SPECS_LEN; i++ )
    {
        layout::PushButton *pBtn = new layout::PushButton( (layout::Window *) dialog );
        pBtn->SetText( rtl::OUString::createFromAscii( WIDGETS_SPECS[ i ].pLabel ) );
        pBtn->SetClickHdl( LINK( this, EditorImpl, CreateWidgetHdl ) );
        if ( WIDGETS_SPECS[ i ].pIconName != NULL )
        {
            rtl::OString aPath ("res/commandimagelist/");
            aPath += WIDGETS_SPECS[ i ].pIconName;
            layout::Image aImg( aPath );
            pBtn->SetModeImage( aImg );
            pBtn->SetImageAlign( IMAGEALIGN_LEFT );
        }
        pBtn->Show();
        maCreateButtons.push_back( pBtn );
        layout::Container *pBox = WIDGETS_SPECS[ i ].bIsContainer ? &aContainers : &aWidgets;
        pBox->Add( pBtn );
    }

#ifdef FILEDLG
    fprintf(stderr,"creating file dialog\n");
    pImportDialog = new FileDialog( NULL/*(layout::Window *) dialog*/, 0 );
    fprintf(stderr,"connecting it\n");
    pImportDialog->SetFileSelectHdl( LINK( this, EditorImpl, ImportDialogHdl ) );
    fprintf(stderr,"done file dialog\n");
#endif

/*    pImportButton = new layout::PushButton( dialog, "import-button" );
    pImportButton->SetClickHdl( LINK( this, EditorImpl, ImportButtonHdl ) );*/
    pExportButton = new layout::PushButton( dialog, "export-button" );
    pExportButton->SetClickHdl( LINK( this, EditorImpl, ExportButtonHdl ) );
}

EditorImpl::~EditorImpl()
{
    delete mpPropertiesList;
    delete mpLayoutTree;
    for ( std::list< layout::PushButton * >::const_iterator i = maCreateButtons.begin();
          i != maCreateButtons.end(); i++)
        delete *i;
    delete pImportButton;
    delete pExportButton;
#ifdef FILEDLG
    delete pImportDialog;
#endif
}

void EditorImpl::loadFile( const rtl::OUString &aTestFile )
{
    fprintf( stderr, "TEST: layout instance\n" );
    uno::Reference< awt::XLayoutRoot > xRoot
        ( new EditorRoot( mxFactory, mpLayoutTree->mpRootWidget ) );

/*
  mxMSF->createInstance
  ( ::rtl::OUString::createFromAscii( "com.sun.star.awt.Layout" ) ),
  uno::UNO_QUERY );
*/
    if ( !xRoot.is() )
    {
        throw uno::RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("could not create awt Layout component!") ),
            uno::Reference< uno::XInterface >() );
    }

#if DEBUG_PRINT
    fprintf( stderr, "TEST: initing root\n" );
#endif

    uno::Reference< lang::XInitialization > xInit( xRoot, uno::UNO_QUERY );
    if ( !xInit.is() )
    {
        throw uno::RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Layout has no XInitialization!") ),
            uno::Reference< uno::XInterface >() );
    }

#if DEBUG_PRINT
    fprintf( stderr, "TEST: running parser\n" );
#endif
    uno::Sequence< uno::Any > aParams( 1 );
    aParams[0] <<= aTestFile;
#if DEBUG_PRINT
    fprintf( stderr, "TEST: do it\n" );
#endif
    xInit->initialize( aParams );
#if DEBUG_PRINT
    fprintf( stderr, "TEST: file loaded\n" );
#endif

    mpLayoutTree->rebuild();
}

void EditorImpl::createWidget( const char *name )
{
    Widget *pWidget = mpLayoutTree->getSelectedWidget();

    Widget *pChild = new Widget( rtl::OUString(), mxToolkit, uno::Reference< awt::XLayoutContainer >( mxToplevel, uno::UNO_QUERY ), rtl::OUString::createFromAscii( name ), awt::WindowAttribute::SHOW );
    if ( !pWidget->addChild( pChild ) )
    {
        delete pChild;
        // we may want to popup an error message
    }
    else
    {
        mpLayoutTree->rebuild();
        mpLayoutTree->selectWidget( pWidget );
    }
}

void EditorImpl::widgetSelected( Widget *pWidget )
{
    // we know can't add widget to a non-container, so let's disable the create
    // buttons then. Would be nice to have a method to check if a container is
    // full as well...
    if ( !pWidget || pWidget->isContainer() )
    {
        for ( std::list< layout::PushButton *>::const_iterator it = maCreateButtons.begin();
              it != maCreateButtons.end(); it++)
            (*it)->Enable();
    }
    else
    {
        for ( std::list< layout::PushButton *>::const_iterator it = maCreateButtons.begin();
              it != maCreateButtons.end(); it++)
            (*it)->Disable();
    }

    mpPropertiesList->selectedWidget( pWidget );
}

IMPL_LINK( EditorImpl, CreateWidgetHdl, layout::Button *, pBtn )
{
    int i = 0;
    for ( std::list< layout::PushButton *>::const_iterator it = maCreateButtons.begin();
          it != maCreateButtons.end(); it++, i++ )
    {
        if ( pBtn == *it )
            break;
    }
    OSL_ASSERT( i < WIDGETS_SPECS_LEN );
    createWidget( WIDGETS_SPECS[i].pName );
    return 0;
}

IMPL_LINK( EditorImpl, ImportButtonHdl, layout::PushButton *, pBtn )
{
    (void) pBtn;
#if DEBUG_PRINT
    fprintf(stderr, "IMPORT!\n");
#endif
#ifdef FILEDLG
    pImportDialog->Execute();
#endif

    return 0;
}

#ifdef FILEDLG
IMPL_LINK( EditorImpl, ImportDialogHdl, FileDialog *, pDialog )
{
    UniString path = pDialog->GetPath();
//fprintf(stderr, "Executing import dialog!\n");

#if DEBUG_PRINT
    fprintf(stderr, "got import file: %s\n",rtl::OUStringToOString( path, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif

    return 0;
}
#endif

IMPL_LINK( EditorImpl, ExportButtonHdl, layout::PushButton *, pBtn )
{
    (void) pBtn;
    mpLayoutTree->print();
    return 0;
}

//** Editor, the Dialog

Editor::Editor( uno::Reference< lang::XMultiServiceFactory > xFactory,
                rtl::OUString aFile )
    : layout::Dialog( (Window*) (NULL), "editor.xml", "dialog" )
    , mpImpl( new EditorImpl( this, xFactory ) )
{
    if ( aFile.getLength() )
        mpImpl->loadFile( aFile );

    // parent:
    FreeResource();
}

Editor::~Editor()
{
    delete mpImpl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
