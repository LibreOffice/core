/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmgroup.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:16:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

DECLARE_LIST( FmCtrlModelList, XInterface* );

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
    FmFormControlMap        aControlMap;

    friend Reflection*      FmXGroupModel_getReflection();
    FmXGroupModel_Impl*     pGroupModelData;            // Properties

public:
    FmXGroupModel();
    virtual ~FmXGroupModel();

    // UNO Anbindung
    SMART_UNO_DECLARATION( FmXGroupModel, FmXControlModel );
    virtual XInterface*     queryInterface(UsrUik);
    virtual XIdlClassRef    getIdlClass();

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
    virtual void addPropertyChangeListener( const XubString& aPropertyName, const XPropertyChangeListenerRef& aListener );
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


#endif // _SVX_FMGROUP_HXX

