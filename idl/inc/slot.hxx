/*************************************************************************
 *
 *  $RCSfile: slot.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#ifndef _SLOT_HXX
#define _SLOT_HXX

#include <types.hxx>
#include <command.hxx>

/******************** class SvMetaSlot **********************************/
//class SbxObject;
class SvMetaSlotMemberList;
class SvMetaSlot : public SvMetaAttribute
{
    SvMetaTypeRef   aSlotType;
    SvMetaAttributeRef aMethod;
    SvIdentifier    aGroupId;
    SvBOOL          aHasCoreId;
    SvIdentifier    aConfigId;
    SvIdentifier    aExecMethod;
    SvIdentifier    aStateMethod;
    SvIdentifier    aDefault;
    SvBOOL          aPseudoSlots;
    SvBOOL          aGet;
    SvBOOL          aSet;

    SvBOOL          aCachable;  // exclusiv
    SvBOOL          aVolatile;
    SvBOOL          aToggle;
    SvBOOL          aAutoUpdate;

    SvBOOL          aSynchron;  // exclusiv
    SvBOOL          aAsynchron;

    SvBOOL          aRecordPerItem;// exclusiv
    SvBOOL          aRecordPerSet;
    SvBOOL          aRecordManual;
    SvBOOL          aNoRecord;
    SvBOOL          aRecordAbsolute;

    SvBOOL          aHasDialog;
    SvBOOL          aMenuConfig;
    SvBOOL          aToolBoxConfig;
    SvBOOL          aStatusBarConfig;
    SvBOOL          aAccelConfig;
    SvBOOL          aFastCall;
    SvBOOL          aContainer;
    SvBOOL          aPlugComm;
    SvIdentifier    aPseudoPrefix;
    SvString        aDisableFlags;
    SvMetaSlot*     pLinkedSlot;
    SvMetaSlot*     pNextSlot;
    ULONG           nListPos;
    SvMetaEnumValue* pEnumValue;
    SvString    aUnoName;

#ifdef IDL_COMPILER
    void            WriteSlot( const ByteString & rShellName,
                            USHORT nCount, const ByteString & rSlotId,
                            const ByteString & rValEnum,
                            SvSlotElementList &rList,
                            const ByteString & rPrefix,
                               SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void    Write( SvIdlDataBase & rBase,
                            SvStream & rOutStm, USHORT nTab,
                             WriteType, WriteAttribute = 0 );

    void            SetEnumValue(SvMetaEnumValue *p)
                    { pEnumValue = p; }
#endif
protected:
    void    SetCachable( BOOL bSet )
            {
                aCachable = bSet;
                if( bSet )
                    aVolatile = FALSE;
            }
    void    SetVolatile( BOOL bSet )
            {
                aVolatile = bSet;
                if( bSet )
                    aCachable = FALSE;
            }
    void    SetToggle( BOOL bSet )
            {
                aToggle = bSet;
            }
    void    SetAutoUpdate( BOOL bSet )
            {
                aAutoUpdate = bSet;
            }

    void    SetSynchron( BOOL bSet )
            {
                aSynchron = bSet;
                if( bSet )
                    aAsynchron = FALSE;
            }
    void    SetAsynchron( BOOL bSet )
            {
                aAsynchron = bSet;
                if( bSet )
                    aSynchron = FALSE;
            }

    void    SetRecordPerItem( BOOL bSet )
            {
                aRecordPerItem = bSet;
                if( bSet )
                    aRecordPerSet = aRecordManual = aNoRecord = FALSE;
            }
    void    SetRecordPerSet( BOOL bSet )
            {
                aRecordPerSet = bSet;
                if( bSet )
                    aRecordPerItem = aRecordManual = aNoRecord = FALSE;
            }
    void    SetRecordManual( BOOL bSet )
            {
                aRecordManual = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = aNoRecord = FALSE;
            }
    void    SetNoRecord( BOOL bSet )
            {
                aNoRecord = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = aRecordManual = FALSE;
            }
    void    SetRecordAbsolute( BOOL bSet )
            { aRecordAbsolute = bSet; }
    void    SetHasDialog( BOOL bSet )
            { aHasDialog = bSet; }
    void    SetMenuConfig( BOOL bSet )
            { aMenuConfig = bSet; }
    void    SetToolBoxConfig( BOOL bSet )
            { aToolBoxConfig = bSet; }
    void    SetStatusBarConfig( BOOL bSet )
            { aStatusBarConfig = bSet; }
    void    SetAccelConfig( BOOL bSet )
            { aAccelConfig = bSet; }
    void    SetAllConfig( BOOL bSet )
            {
                aMenuConfig     = bSet;
                aToolBoxConfig  = bSet;
                aStatusBarConfig = bSet;
                aAccelConfig    = bSet;
            }
    void    SetFastCall( BOOL bSet )
            { aFastCall = bSet; }
    void    SetContainer( BOOL bSet )
            { aContainer = bSet; }
    void    SetPlugComm( BOOL bSet )
            { aPlugComm = bSet; }

public:
            SV_DECL_META_FACTORY1( SvMetaSlot, SvMetaReference, 11 )
            SvMetaSlot();
            SvMetaSlot( SvMetaType * pType );

    virtual BOOL    IsVariable() const;
    virtual BOOL    IsMethod() const;
    virtual ByteString  GetMangleName( BOOL bVariable ) const;

    SvMetaAttribute *   GetMethod() const;
    SvMetaType *        GetSlotType() const;
    BOOL                GetHasCoreId() const;
    const ByteString &      GetGroupId() const;
    const ByteString &      GetConfigId() const;
    const ByteString &      GetExecMethod() const;
    const ByteString &      GetStateMethod() const;
    const ByteString &      GetDefault() const;
    const ByteString &      GetDisableFlags() const;
    BOOL                GetPseudoSlots() const;
    /*
    BOOL                GetGet() const;
    BOOL                GetSet() const;
    */
    BOOL                GetCachable() const;
    BOOL                GetVolatile() const;
    BOOL                GetToggle() const;
    BOOL                GetAutoUpdate() const;

    BOOL                GetSynchron() const;
    BOOL                GetAsynchron() const;

    BOOL                GetRecordPerItem() const;
    BOOL                GetRecordPerSet() const;
    BOOL                GetRecordManual() const;
    BOOL                GetNoRecord() const;
    BOOL                GetRecordAbsolute() const;

    BOOL                GetHasDialog() const;
    const ByteString &      GetPseudoPrefix() const;
    const ByteString &      GetUnoName() const;
    BOOL                GetMenuConfig() const;
    BOOL                GetToolBoxConfig() const;
    BOOL                GetStatusBarConfig() const;
    BOOL                GetAccelConfig() const;
    BOOL                GetFastCall() const;
    BOOL                GetContainer() const;
    BOOL                GetPlugComm() const;
    SvMetaSlot*         GetLinkedSlot() const
                        { return pLinkedSlot; }
    SvMetaSlot*         GetNextSlot() const
                        { return pNextSlot; }
    ULONG               GetListPos() const
                        { return nListPos; }
    void                SetListPos(ULONG n)
                        { nListPos = n; }
    void                ResetSlotPointer()
                        { pNextSlot = pLinkedSlot = 0; }

//    void                FillSbxObject( SvIdlDataBase & rBase, SbxObject *, BOOL bProp );
#ifdef IDL_COMPILER
    SvMetaEnumValue*    GetEnumValue() const
                        { return pEnumValue; }
    virtual BOOL        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm );
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                              SvStream & rOutStm, USHORT nTab );
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
    virtual void        Insert( SvSlotElementList&, const ByteString & rPrefix,
                                SvIdlDataBase& );
    void                WriteSlotStubs( const ByteString & rShellName,
                                    ByteStringList & rList,
                                    SvStream & rOutStm );
    USHORT              WriteSlotMap( const ByteString & rShellName,
                                    USHORT nCount,
                                    SvSlotElementList&,
                                    const ByteString &,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );
    USHORT              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvStream & rOutStm );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable );
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& );
    void                CompareSlotAttributes( SvMetaSlot *pOther );
#endif
};
SV_DECL_IMPL_REF(SvMetaSlot)
SV_DECL_IMPL_PERSIST_LIST(SvMetaSlot,SvMetaSlot *)

#endif // _SLOT_HXX
