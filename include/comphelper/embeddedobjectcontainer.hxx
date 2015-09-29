/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef INCLUDED_COMPHELPER_EMBEDDEDOBJECTCONTAINER_HXX
#define INCLUDED_COMPHELPER_EMBEDDEDOBJECTCONTAINER_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/comphelperdllapi.h>

#include <rtl/ustring.hxx>

namespace comphelper
{
    class EmbeddedObjectContainer;
    /** Helper interface to give access to some common object which replace the SfxObjectShell
    */
    class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI IEmbeddedHelper
    {
    public:
        virtual EmbeddedObjectContainer& getEmbeddedObjectContainer() const = 0;
        virtual com::sun::star::uno::Reference < com::sun::star::embed::XStorage > getStorage() const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > getInteractionHandler() const = 0;
        virtual bool isEnableSetModified() const = 0;

    protected:
        ~IEmbeddedHelper() {}
    };

struct EmbedImpl;
class COMPHELPER_DLLPUBLIC EmbeddedObjectContainer
{
    EmbedImpl*  pImpl;

    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject > Get_Impl( const OUString&,
            const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xCopy);

public:
    // add an embedded object to the container storage
    bool StoreEmbeddedObject(
        const css::uno::Reference<css::embed::XEmbeddedObject>& xObj, OUString& rName, bool bCopy,
        const OUString& rSrcShellID, const OUString& rDestShellID );

    // add an embedded object that has been imported from the container storage - should only be called by filters!
    void                AddEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, const OUString& );

                        EmbeddedObjectContainer();
                        EmbeddedObjectContainer( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& );
                        EmbeddedObjectContainer( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                                                 const com::sun::star::uno::Reference < com::sun::star::uno::XInterface >& );
                        ~EmbeddedObjectContainer();

    void                SwitchPersistence( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& );
    bool            CommitImageSubStorage();
    void                ReleaseImageSubStorage();

    OUString     CreateUniqueObjectName();

    // get a list of object names that have been added so far
    com::sun::star::uno::Sequence < OUString > GetObjectNames();

    // check for existence of objects at all
    bool            HasEmbeddedObjects();

    // check existence of an object - either by identity or by name
    bool            HasEmbeddedObject( const OUString& );
    bool            HasEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );
    bool            HasInstantiatedEmbeddedObject( const OUString& );

    // get the object name of an object - this is the persist name if the object has persistence
    OUString     GetEmbeddedObjectName( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );

    // retrieve an embedded object by name that either has been added already or is available in the container storage
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject > GetEmbeddedObject( const OUString& );

    // create an object from a ClassId
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        CreateEmbeddedObject( const com::sun::star::uno::Sequence < sal_Int8 >&, OUString& );

    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        CreateEmbeddedObject( const com::sun::star::uno::Sequence < sal_Int8 >&,
                        const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >&, OUString& );

    // insert an embedded object into the container - objects persistent representation will be added to the storage
    bool            InsertEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, OUString& );

    // load an embedded object from a MediaDescriptor and insert it into the container
    // a new object will be created from the new content and returned
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        InsertEmbeddedObject( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, OUString& );

    // create an embedded link based on a MediaDescriptor and insert it into the container
    // a new object will be created from the new content and returned
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        InsertEmbeddedLink( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, OUString& );

    // create an object from a stream that contains its persistent representation and insert it as usual (usually called from clipboard)
    // a new object will be created from the new content and returned
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >
                        InsertEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >&, OUString& );

    // copy an embedded object into the storage, open the new copy and return it
    css::uno::Reference <css::embed::XEmbeddedObject> CopyAndGetEmbeddedObject(
        EmbeddedObjectContainer& rSrc, const css::uno::Reference <css::embed::XEmbeddedObject>& xObj, OUString& rName,
        const OUString& rSrcShellID, const OUString& rDestShellID );

    // move an embedded object from one container to another one
    bool MoveEmbeddedObject( EmbeddedObjectContainer& rSrc, const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, OUString& );

    // remove an embedded object from the container and from the storage; if object can't be closed
    // #i119941, bKeepToTempStorage: use to specify whether store the removed object to temporary storage+
    bool            RemoveEmbeddedObject( const OUString& rName, bool bClose = true, bool bKeepToTempStorage = true );
    bool            RemoveEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, bool bClose = true, bool bKeepToTempStorage = true );

    // close and remove an embedded object from the container without removing it from the storage
    bool            CloseEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );

    // move an embedded object to another container (keep the persistent name)
    bool            MoveEmbeddedObject( const OUString& rName, EmbeddedObjectContainer& );

    // get the stored representation for the object
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > GetObjectStream( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, OUString* pMediaType=0 );
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > GetObjectStream( const OUString& aName, OUString* pMediaType );

    // get the stored graphical representation for the object
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > GetGraphicStream( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&, OUString* pMediaType=0 );

    // get the stored graphical representation by the object name
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > GetGraphicStream( const OUString& aName, OUString* pMediaType=0 );

    // add a graphical representation for an object
    bool            InsertGraphicStream( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const OUString& rObjectName, const OUString& rMediaType );

    // try to add a graphical representation for an object in optimized way ( might fail )
    bool            InsertGraphicStreamDirectly( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const OUString& rObjectName, const OUString& rMediaType );

    // remove a graphical representation for an object
    bool            RemoveGraphicStream( const OUString& rObjectName );

    // copy the graphical representation from different container
    bool            TryToCopyGraphReplacement( EmbeddedObjectContainer& rSrc,
                                                    const OUString& aOrigName,
                                                    const OUString& aTargetName );

    void                CloseEmbeddedObjects();
    bool            StoreChildren(bool _bOasisFormat,bool _bObjectsOnly);
    bool            StoreAsChildren( bool _bOasisFormat
                                        ,bool _bCreateEmbedded
                                        ,const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& _xStorage);

    static com::sun::star::uno::Reference< com::sun::star::io::XInputStream > GetGraphicReplacementStream(
                                            sal_Int64 nViewAspect,
                                            const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >&,
                                            OUString* pMediaType );

    /** call setPersistentEntry for each embedded object in the container
    *
    * \param _xStorage The storage where to store the objects.
    * \param _bClearModifedFlag If <TRUE/> then the modified flag will be set to <FALSE/> otherwise nothing happen.
    * \return <FALSE/> if no error occurred, otherwise <TRUE/>.
    */
    bool             SetPersistentEntries(const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& _xStorage,bool _bClearModifedFlag = true);

    bool getUserAllowsLinkUpdate() const;
    void setUserAllowsLinkUpdate(bool bNew);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
