/*************************************************************************
 *
 *  $RCSfile: unoevent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:45:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNOEVENT_HXX
#define _UNOEVENT_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif

class SvxMacroTableDtor;
class SvxMacroItem;
class SvxMacro;
class SwXFrame;
class SwXTextFrame;
class SwXTextGraphicObject;
class SwXTextEmbeddedObject;
class SwXFrameStyle;
class SwFmtINetFmt;

/**
 * SwBaseEventDescriptor: Abstract class that implements the basics
 * of an XNameReplace that is delivered by the
 * XEventsSupplier::getEvents() method.
 *
 * The functionality this class provides is:
 * 1) Which elements are in the XNameReplace?
 * 2) Mapping from Api names to item IDs.
 * 3) conversion from SvxMacroItem to Any and vice versa.
 *
 * All details of how to actually get and set SvxMacroItem(s) have to
 * be supplied by the base class.
 */
class SwBaseEventDescriptor : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XNameReplace,
    ::com::sun::star::lang::XServiceInfo
>
{
    const ::rtl::OUString sEventType;
    const ::rtl::OUString sMacroName;
    const ::rtl::OUString sLibrary;
    const ::rtl::OUString sStarBasic;
    const ::rtl::OUString sJavaScript;
    const ::rtl::OUString sScript;
    const ::rtl::OUString sNone;
    const ::rtl::OUString sEmpty;


    /// name of own service
    const ::rtl::OUString sServiceName;

protected:
    /// an empty macro
    const SvxMacro aEmptyMacro;

    /// last element is 0
    const USHORT* aSupportedMacroItemIDs;

public:

     SwBaseEventDescriptor(const USHORT* aMacroItems);

    virtual ~SwBaseEventDescriptor();


    // XNameReplace
    /// calls replaceByName(const USHORT, const SvxMacro&)
    virtual void SAL_CALL replaceByName(
        const ::rtl::OUString& rName,                /// API name of event
        const ::com::sun::star::uno::Any& rElement ) /// event (PropertyValues)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XNameAccess (via XNameReplace)
    /// calls getByName(USHORT)
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(
        const ::rtl::OUString& rName )  /// API name of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    // XNameAxcess (via XNameReplace)
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                                        getElementNames()
        throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasByName(
        const ::rtl::OUString& rName )
            throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess (via XNameReplace)
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasElements()
        throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    /// must be implemented in subclass
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException ) = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getSupportedServiceNames(void)
            throw( ::com::sun::star::uno::RuntimeException );

protected:

    /// Must be implemented in subclass.
    virtual void replaceByName(
        const USHORT nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException) = 0;

    /// Must be implemented in subclass.
    virtual void getByName(
        SvxMacro& rMacro,
        const USHORT nEvent )
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException) = 0;

    /// convert an API event name to the event ID as used by SvxMacroItem
    USHORT mapNameToEventID(const ::rtl::OUString& rName) const;

    /// convert an event ID to an API event name
    ::rtl::OUString mapEventIDToName(USHORT nPoolID) const;

    /// get the event ID for the name; return 0 if not supported
    USHORT getMacroID(const ::rtl::OUString& rName) const;

    /// create PropertyValues and Any from macro
    void getAnyFromMacro(
        ::com::sun::star::uno::Any& aAny,   // Any to be filled by Macro values
        const SvxMacro& rMacro);

    /// create macro from PropertyValues (in an Any)
    void getMacroFromAny(
        SvxMacro& aMacro,       // reference to be filled by Any
        const ::com::sun::star::uno::Any& rAny)
            throw (
                ::com::sun::star::lang::IllegalArgumentException);

};




/**
 * SwEventDescriptor: Implement the XNameReplace that is delivered by
 * the XEventsSupplier::getEvents() method. The SwEventDescriptor has
 * to be subclassed to implement the events for a specific
 * objects. The subclass has to
 * 1) supply the super class constructor with a list of known events (item IDs)
 * 2) supply the super class constructor with a reference of it's parent object
 *    (to prevent destruction)
 * 3) implement getItem() and setItem(...) methods.
 *
 * If no object is available to which the SwEventDescriptor can attach itself,
 * the class SwDetachedEventDescriptor should be used.
 */
class SwEventDescriptor : public SwBaseEventDescriptor
{
    /// keep reference to parent to prevent it from being destroyed
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > xParentRef;

public:

     SwEventDescriptor(::com::sun::star::uno::XInterface& rParent,
                      const USHORT* aMacroItems);

    virtual ~SwEventDescriptor();


protected:


    virtual void replaceByName(
        const USHORT nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void getByName(
        SvxMacro& rMacros,          /// macro to be filled with values
        const USHORT nEvent )       /// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);


    /// Get the SvxMacroItem from the parent.
    /// must be implemented by subclass
    virtual const SvxMacroItem& getMacroItem() = 0;

    /// Set the SvxMacroItem at the parent.
    /// must be implemented by subclass
    virtual void setMacroItem(const SvxMacroItem& rItem) = 0;
};


/**
 * SwDetachedEventDescriptor:
 */
class SwDetachedEventDescriptor : public SwBaseEventDescriptor
{
    // the macros; aMacros[i] is the value for aSupportedMacroItemIDs[i]
    SvxMacro** aMacros;

    const ::rtl::OUString sImplName;

public:

     SwDetachedEventDescriptor(const USHORT* aMacroItems);

    virtual ~SwDetachedEventDescriptor();

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:

    sal_Int16 getIndex(const USHORT nID);

    virtual void replaceByName(
        const USHORT nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void getByName(
        SvxMacro& rMacro,           /// macro to be filled
        const USHORT nEvent )       /// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    /// do we have an event?
    /// return sal_True: we have a macro for the event
    /// return sal_False: no macro; getByName() will return an empty macro
    /// IllegalArgumentException: the event is not supported
    virtual const sal_Bool hasByName(
        const USHORT nEvent )       /// item ID of event
             throw(
                ::com::sun::star::lang::IllegalArgumentException);

};

class SwHyperlinkEventDescriptor : public SwDetachedEventDescriptor
{
    const ::rtl::OUString sImplName;

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

public:

     SwHyperlinkEventDescriptor();

    virtual ~SwHyperlinkEventDescriptor();

    void copyMacrosFromINetFmt(const SwFmtINetFmt& aFmt);
    void copyMacrosIntoINetFmt(SwFmtINetFmt& aFmt);

    void copyMacrosFromNameReplace(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameReplace> & xReplace);
};


class SwMacroTableEventDescriptor : public SwDetachedEventDescriptor
{
public:

     SwMacroTableEventDescriptor();
     SwMacroTableEventDescriptor(const SvxMacroTableDtor& aFmt);

    virtual ~SwMacroTableEventDescriptor();

    void copyMacrosFromTable(const SvxMacroTableDtor& aFmt);
    void copyMacrosIntoTable(SvxMacroTableDtor& aFmt);
};


//
// classes that implement the connection to specific UNO objects
//


// SwEventDescriptor for
// 1) SwXTextFrame
// 2) SwXGraphicObject
// 3) SwXEmbeddedObject
// All these objects are an SwXFrame, so they can use a common implementation
class SwFrameEventDescriptor : public SwEventDescriptor
{
    ::rtl::OUString sSwFrameEventDescriptor;

    SwXFrame& rFrame;

public:
    SwFrameEventDescriptor( SwXTextFrame& rFrameRef );
    SwFrameEventDescriptor( SwXTextGraphicObject& rGraphicRef );
    SwFrameEventDescriptor( SwXTextEmbeddedObject& rObjectRef );

    ~SwFrameEventDescriptor();

    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
};

class SwFrameStyleEventDescriptor : public SwEventDescriptor
{
    ::rtl::OUString sSwFrameStyleEventDescriptor;

    SwXFrameStyle& rStyle;

public:
    SwFrameStyleEventDescriptor( SwXFrameStyle& rStyleRef );

    ~SwFrameStyleEventDescriptor();

    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
};


#endif
