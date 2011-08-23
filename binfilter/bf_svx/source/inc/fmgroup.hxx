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
#ifndef _SVX_FMGROUP_HXX
#define _SVX_FMGROUP_HXX

#ifndef _SVX_FMCPONT_HXX
#include "fmcpont.hxx"
#endif

#ifndef __DBFORM_HXX
#include <dbform.hxx>
#endif
namespace binfilter {

DECLARE_LIST( FmCtrlModelList, XInterface* )//STRIP008 DECLARE_LIST( FmCtrlModelList, XInterface* );

struct FmXGroupModel_Impl;

//==================================================================
// FmXGroupModel
//==================================================================
class FmXGroupModel : public XBoundControl,
                      public XLoadListener,
                      public XPropertyChangeListener,
                      public XGroup,
                      public FmXControlModel
{
    FmFormControlMap		aControlMap;

    friend Reflection*		FmXGroupModel_getReflection();
    FmXGroupModel_Impl*		pGroupModelData;			// Properties

public:
    FmXGroupModel();
    virtual ~FmXGroupModel();

    // UNO Anbindung
    SMART_UNO_DECLARATION( FmXGroupModel, FmXControlModel );
    virtual XInterface*		queryInterface(UsrUik);
    virtual XIdlClassRef	getIdlClass();

    // XGroup
    virtual INT32 getFormControlCount();
    virtual XFormControlRef getFormControlByName( const XubString& sName ) const;
    virtual XFormControlRef getFormControlByIndex( INT32 Index ) const;
    virtual void appendFormControl( const XFormControlRef& FormControl );
    virtual XFormControlRef removeFormControl( const XFormControlRef& FormControl );

    // XFormControl
    virtual void setParent( const XFormRef& Parent );

    // XEventListener
    virtual void disposing( const EventObject& Source );

    // XPropertiesChangeListener
    virtual void propertyChange( const PropertyChangeEvent& evt );

    // XLoadListener
    virtual void loaded( const EventObject& rEvent );
    virtual void unloaded( const EventObject& rEvent );

    // XBoundControl
    virtual void addBoundControlListener( const XBoundControlListenerRef& l );
    virtual void removeBoundControlListener( const XBoundControlListenerRef& l );

    // XPersistObject
    virtual XubString getServiceName( void ) const;
    virtual void write( const XObjectOutputStreamRef& OutStream );
    virtual void read( const XObjectInputStreamRef& InStream );

    // PropertySetInterface
    virtual FmXPropertySetInfo* createPropertySetInfo() const;
    virtual BOOL setPropertyValue( UINT16 nId, const XubString& aPropertyName,
                                   const UsrAny& aValue,
                                   PropertyChangeEventSequence* pSeq,
                                   INT32 nIndex );
    virtual UsrAny getPropertyValue( UINT16 nId, const XubString& aPropertyName ) const;
    virtual	void addPropertyChangeListener( const XubString& aPropertyName, const XPropertyChangeListenerRef& aListener );
    virtual void removePropertyChangeListener( const XubString& aPropertyName, const XPropertyChangeListenerRef& aListener );
};

//==================================================================
// FmXGroupModelInfo
// Properties:
//==================================================================
class FmXGroupModelInfo : public FmXControlModelInfo
{
    friend class FmXGroupModel;

protected:
    FmXGroupModelInfo( const FmXGroupModel* pCp );
    virtual void fillProperties( UINT32& nIndex, PropertySequence& aSeq ) const;
};


}//end of namespace binfilter
#endif // _SVX_FMGROUP_HXX

