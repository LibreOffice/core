/*************************************************************************
 *
 *  $RCSfile: basmgr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ab $ $Date: 2001-02-26 11:38:11 $
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

#ifndef _SFX_BASMGR_HXX
#define _SFX_BASMGR_HXX

#ifndef _BASMGR_HXX_
#include <basic/basmgr.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XSTARBASICACCESS_HPP_
#include <com/sun/star/script/XStarBasicAccess.hpp>
#endif


// Basic XML Import/Export
com::sun::star::uno::Reference< com::sun::star::script::XStarBasicAccess >
    getStarBasicAccess( BasicManager* pMgr );


class SfxDialogContainer;

class SfxBasicManager : public BasicManager
{
    friend class LibraryContainer_Impl;

private:
    BasicLibs*          pLibs;
    BasicErrorManager*  pErrorMgr;

    String              aName;
    String              aStorageName;
    BOOL                bBasMgrModified;

    SfxDialogContainer* mpDialogContainer;

    void                Init();

protected:
    BOOL            ImpStoreLibary( StarBASIC* pLib, SvStorage& rStorage ) const;
    BOOL            ImpLoadLibary( BasicLibInfo* pLibInfo ) const;
    BOOL            ImpLoadLibary( BasicLibInfo* pLibInfo, SvStorage* pCurStorage, BOOL bInfosOnly = FALSE ) const;
    void            ImpCreateStdLib( StarBASIC* pParentFromStdLib );
    void            ImpMgrNotLoaded(  const String& rStorageName  );
    BasicLibInfo*   CreateLibInfo();
    void            LoadBasicManager( SvStorage& rStorage, BOOL bLoadBasics = TRUE );
    void            LoadOldBasicManager( SvStorage& rStorage );
    BOOL            ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const;
    void            ImplGetPassword( USHORT nLib ) const;
    BOOL            ImplEncryptStream( SvStream& rStream ) const;
    BasicLibInfo*   FindLibInfo( StarBASIC* pBasic ) const;
    void            CheckModules( StarBASIC* pBasic, BOOL bReference ) const;
    void            SetFlagToAllLibs( short nFlag, BOOL bSet ) const;
                    SfxBasicManager();  // Nur zum anpassen von Pfaden bei 'Speichern unter'.

public:
                    TYPEINFO();
                    SfxBasicManager( SvStorage& rStorage, StarBASIC* pParentFromStdLib = NULL, String* pLibPath = NULL );
                    SfxBasicManager( StarBASIC* pStdLib, String* pLibPath = NULL );
                    ~SfxBasicManager();

    void            SetStorageName( const String& rName )   { aStorageName = rName; }
    String          GetStorageName() const                  { return aStorageName; }
    void            SetName( const String& rName )          { aName = rName; }
    String          GetName() const                         { return aName; }


    static BOOL     HasBasicManager( const SvStorage& rStorage );
    static BOOL     CopyBasicData( SvStorage* pFrom, const String& rSourceURL, SvStorage* pTo);
    void            Merge(  SvStorage& rFromStorage );

    USHORT          GetLibCount() const;
    StarBASIC*      GetStdLib() const;
    StarBASIC*      GetLib( USHORT nLib ) const;
    StarBASIC*      GetLib( const String& rName ) const;
    USHORT          GetLibId( const String& rName ) const;
    USHORT          GetLibId( StarBASIC* pLib ) const;
    BOOL            HasLib( const String& rName ) const;

    void            Store( SvStorage& rStorage );
    void            Store( SvStorage& rStorage, BOOL bStoreLibs );

    BOOL            SetLibName( USHORT nLib, const String& rName );
    String          GetLibName( USHORT nLib );

    void            SetDialogContainer( SfxDialogContainer* pDialogContainer );

    BOOL            SetLibStorageName( USHORT nLib, const String& rName );
    String          GetLibStorageName( USHORT nLib );
    String          GetRelLibStorageName( USHORT nLib );

    BOOL            IsLibLoaded( USHORT nLib ) const;
    BOOL            LoadLib( USHORT nLib );
    BOOL            UnloadLib( USHORT nLib );
    BOOL            StoreLib( USHORT nLib ) const;
    BOOL            RemoveLib( USHORT nLib );
    BOOL            RemoveLib( USHORT nLib, BOOL bDelBasicFromStorage );

    BOOL            IsReference( USHORT nLib );
    BOOL            IsExtern( USHORT nLib );

    StarBASIC*      CreateLib( const String& rLibName );
    // For XML import/export:
    StarBASIC*      CreateLib( const String& rLibName, const String& Password,
                               const String& ExternalSourceURL, const String& LinkTargetURL );
    StarBASIC*      AddLib( SvStorage& rStorage, const String& rLibName, BOOL bReference );
    void            AddLib( StarBASIC* pLib );
    BOOL            MoveLib( USHORT nLib, USHORT nNewPos );

    BOOL            HasPassword( USHORT nLib ) const;
    String          GetPassword( USHORT nLib ) const;
    void            SetPassword( USHORT nLib, const String& rNewPassword );

    // Der BasicManager gibt die Basics auch raus, wenn das Passwort nicht
    // geprueft wurde, da man auch ohne Passwort mit der Lib arbeiten kann.
    // Es ist Sache der App, was Sie dem Anwender ohne Passwort nicht gestattet,
    // also z.B. das Betrachten der Source oder das Anzeigen/Loeschen von Modulen.
    BOOL            IsPasswordVerified( USHORT nLib ) const;
    void            SetPasswordVerified( USHORT nLib );


    // Modify-Flag wird nur beim Speichern zurueckgesetzt.
    BOOL            IsModified() const;
    BOOL            IsBasicModified() const;
    BOOL            IsManagerModified() const { return bBasMgrModified; }

    BOOL            HasErrors();
    void            ClearErrors();
    BasicError*     GetFirstError();
    BasicError*     GetNextError();
};

#endif  //_SFX_BASMGR_HXX
