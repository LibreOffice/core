/*************************************************************************
 *
 *  $RCSfile: module.hxx,v $
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

#ifndef _MODULE_HXX
#define _MODULE_HXX

#include <slot.hxx>
#include <object.hxx>

struct SvNamePos
{
    SvGlobalName    aUUId;
    UINT32          nStmPos;
    SvNamePos( const SvGlobalName & rName, UINT32 nPos )
        : aUUId( rName )
        , nStmPos( nPos ) {}
};
DECLARE_LIST( SvNamePosList, SvNamePos *)

/******************** class SvMetaModule *********************************/
class SvMetaModule : public SvMetaExtern
{
    SvMetaClassMemberList       aClassList;
    SvMetaTypeMemberList        aTypeList;
    SvMetaAttributeMemberList   aAttrList;
// Browser
    String                  aIdlFileName;
    SvString                aHelpFileName;
    SvString                aSlotIdFile;
    SvString                aTypeLibFile;
    SvString                aModulePrefix;

#ifdef IDL_COMPILER
    BOOL                    bImported   : 1,
                            bIsModified : 1;
    SvGlobalName            aBeginName;
    SvGlobalName            aEndName;
    SvGlobalName            aNextName;
protected:
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteContextSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
#endif
public:
                        SV_DECL_META_FACTORY1( SvMetaModule, SvMetaExtern, 13 )
                        SvMetaModule();

    const String &      GetIdlFileName() const { return aIdlFileName; }
    const ByteString &      GetModulePrefix() const { return aModulePrefix; }

    virtual BOOL        SetName( const ByteString & rName, SvIdlDataBase * = NULL  );

    const ByteString &      GetHelpFileName() const { return aHelpFileName; }
    const ByteString &      GetTypeLibFileName() const { return aTypeLibFile; }

    const SvMetaAttributeMemberList & GetAttrList() const { return aAttrList; }
    const SvMetaTypeMemberList & GetTypeList() const { return aTypeList; }
    const SvMetaClassMemberList & GetClassList() const { return aClassList; }

#ifdef IDL_COMPILER
                        SvMetaModule( const String & rIdlFileName,
                                      BOOL bImported );

    BOOL                FillNextName( SvGlobalName * );
    BOOL                IsImported() const { return bImported; }
    BOOL                IsModified() const { return bIsModified; }

    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );

    virtual void        WriteAttributes( SvIdlDataBase & rBase,
                                        SvStream & rOutStm, USHORT nTab,
                                            WriteType, WriteAttribute = 0 );
//    virtual void        WriteSbx( SvIdlDataBase & rBase, SvStream & rOutStm, SvNamePosList & rList );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                    WriteType, WriteAttribute = 0 );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void        WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                                    Table* pTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                      Table *pIdTable );

    virtual void        WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
#endif
};
SV_DECL_IMPL_REF(SvMetaModule)
SV_DECL_IMPL_PERSIST_LIST(SvMetaModule,SvMetaModule *)


#endif // _MODULE_HXX

