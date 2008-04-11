/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: embeddedobjectcontainer.hxx,v $
 * $Revision: 1.15 $
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
#ifndef _COMPHELPER_OBJECTCONTAINER_HXX_
#define _COMPHELPER_OBJECTCONTAINER_HXX_

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStrem.hpp>
#endif
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "comphelper/comphelperdllapi.h"

#include <rtl/ustring.hxx>

namespace comphelper
{
    class EmbeddedObjectContainer;
    /** Helper interface to give access to some common object which replace the SfxObjectShell
    */
    class SAL_NO_VTABLE IEmbeddedHelper
    {
    public:
        virtual EmbeddedObjectContainer& getEmbeddedObjectContainer() const = 0;
        virtual com::sun::star::uno::Reference < com::sun::star::embed::XStorage > getStorage() const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > getInteractionHandler() const = 0;
        virtual bool isEnableSetModified() const = 0;
    };

struct EmbedImpl;
class COMPHELPER_DLLPUBLIC EmbeddedObjectContainer
{
    EmbedImpl*  pImpl;

    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject > Get_Impl( const ::rtl::OUString&,
            const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xCopy);

public:
    // add an embedded object to the container storage
    sal_Bool            StoreEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, ::rtl::OUString&, sal_Bool );

    // add an embedded object that has been imported from the container storage - should only be called by filters!
    void                AddEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, const ::rtl::OUString& );

                        EmbeddedObjectContainer();
                        EmbeddedObjectContainer( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& );
                        EmbeddedObjectContainer( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                                 const com::sun::star::uno::Reference < com::sun::star::uno::XInterface >& );
                        ~EmbeddedObjectContainer();

    void                SwitchPersistence( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& );
    sal_Bool            CommitImageSubStorage();
    void                ReleaseImageSubStorage();

    ::rtl::OUString     CreateUniqueObjectName();

    // get a list of object names that have been added so far
    com::sun::star::uno::Sequence < ::rtl::OUString > GetObjectNames();

    // check for existence of objects at all
    sal_Bool            HasEmbeddedObjects();

    // check existence of an object - either by identity or by name
    sal_Bool            HasEmbeddedObject( const ::rtl::OUString& );
    sal_Bool            HasEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );
    sal_Bool            HasInstantiatedEmbeddedObject( const ::rtl::OUString& );

    // get the object name of an object - this is the persist name if the object has persistence
    ::rtl::OUString     GetEmbeddedObjectName( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );

    // retrieve an embedded object by name that either has been added already or is available in the container storage
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject > GetEmbeddedObject( const ::rtl::OUString& );

    // create an object from a ClassId
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        CreateEmbeddedObject( const com::sun::star::uno::Sequence < sal_Int8 >&, ::rtl::OUString& );

    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        CreateEmbeddedObject( const com::sun::star::uno::Sequence < sal_Int8 >&,
                        const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >&, ::rtl::OUString& );

    // insert an embedded object into the container - objects persistant representation will be added to the storage
    sal_Bool            InsertEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, ::rtl::OUString& );

    // load an embedded object from a MediaDescriptor and insert it into the container
    // a new object will be created from the new content and returned
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        InsertEmbeddedObject( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, ::rtl::OUString& );

    // create an embedded link based on a MediaDescriptor and insert it into the container
    // a new object will be created from the new content and returned
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        InsertEmbeddedLink( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, ::rtl::OUString& );

    // create an object from a stream that contains its persistent representation and insert it as usual (usually called from clipboard)
    // a new object will be created from the new content and returned
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        InsertEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >&, ::rtl::OUString& );

    // copy an embedded object into the storage
    sal_Bool CopyEmbeddedObject( EmbeddedObjectContainer& rSrc, const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, ::rtl::OUString& );

    // copy an embedded object into the storage, open the new copy and return it
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject > CopyAndGetEmbeddedObject( EmbeddedObjectContainer& rSrc, const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj, /* TODO const ::rtl::OUString& aOrigName,*/ ::rtl::OUString& rName );

    // move an embedded object from one container to another one
    sal_Bool MoveEmbeddedObject( EmbeddedObjectContainer& rSrc, const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, ::rtl::OUString& );

    // remove an embedded object from the container and from the storage; if object can't be closed
    sal_Bool            RemoveEmbeddedObject( const ::rtl::OUString& rName, sal_Bool bClose=sal_True );
    sal_Bool            RemoveEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, sal_Bool bClose=sal_True );

    // close and remove an embedded object from the container without removing it from the storage
    sal_Bool            CloseEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );

    // move an embedded object to another container (keep the persistent name)
    sal_Bool            MoveEmbeddedObject( const ::rtl::OUString& rName, EmbeddedObjectContainer& );

    // get the stored graphical representation for the object
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > GetGraphicStream( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, ::rtl::OUString* pMediaType=0 );

    // get the stored graphical representation by the object name
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > GetGraphicStream( const ::rtl::OUString& aName, ::rtl::OUString* pMediaType=0 );

    // add a graphical representation for an object
    sal_Bool            InsertGraphicStream( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const ::rtl::OUString& rObjectName, const ::rtl::OUString& rMediaType );

    // try to add a graphical representation for an object in optimized way ( might fail )
    sal_Bool            InsertGraphicStreamDirectly( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const ::rtl::OUString& rObjectName, const rtl::OUString& rMediaType );

    // remove a graphical representation for an object
    sal_Bool            RemoveGraphicStream( const ::rtl::OUString& rObjectName );

    // copy the graphical representation from different container
    sal_Bool            TryToCopyGraphReplacement( EmbeddedObjectContainer& rSrc,
                                                    const ::rtl::OUString& aOrigName,
                                                    const ::rtl::OUString& aTargetName );

    void                CloseEmbeddedObjects();
    sal_Bool            StoreChildren(sal_Bool _bOasisFormat,sal_Bool _bObjectsOnly);
    sal_Bool            StoreAsChildren( sal_Bool _bOasisFormat
                                        ,sal_Bool _bCreateEmbedded
                                        ,const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& _xStorage);

    static com::sun::star::uno::Reference< com::sun::star::io::XInputStream > GetGraphicReplacementStream(
                                            sal_Int64 nViewAspect,
                                            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >&,
                                            ::rtl::OUString* pMediaType );

    /** call setPersistentEntry for each embedded object in the container
    *
    * \param _xStorage The storeage where to store the objects.
    * \param _bClearModifedFlag If <TRUE/> then the modifed flag will be set to <FALSE/> otherwise nothing happen.
    * \return <FALSE/> if no error occured, otherwise <TRUE/>.
    */
    sal_Bool             SetPersistentEntries(const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& _xStorage,bool _bClearModifedFlag = true);
};

}

#endif

