/*************************************************************************
 *
 *  $RCSfile: sfxpicklist.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-08-10 05:29:08 $
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

#ifndef _SFX_PICKLIST_HXX_
#define _SFX_PICKLIST_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#include <vector>

#define PICKLIST_MAXSIZE  10

class SfxPickList : public SfxListener
{
    struct PickListEntry
    {
        PickListEntry( const String& _aName, const String& _aFilter, const String& _aTitle ) :
            aName( _aName ), aFilter( _aFilter ), aTitle( _aTitle ) {}

        String      aName;
        String      aFilter;
        String      aTitle;
        String      aOptions;
    };

    static SfxPickList*             pUniqueInstance;
    static osl::Mutex*              pMutex;

    std::vector< PickListEntry* >   m_aPicklistVector;
    sal_uInt32                      m_nAllowedMenuSize;

                            SfxPickList( ULONG nMenuSize );
                            ~SfxPickList();

    static osl::Mutex*      GetOrCreateMutex();

    String                  CreatePicklistMenuTitle( const String& aURL, sal_uInt32 nNo );
    PickListEntry*          GetPickListEntry( sal_uInt32 nIndex );
    void                    CreatePickListEntries();
    void                    RemovePickListEntries();

    public:
        static SfxPickList* GetOrCreate( const ULONG nMenuSize );
        static SfxPickList* Get();
        static void         Delete();

        sal_uInt32          GetAllowedMenuSize() { return m_nAllowedMenuSize; }
        sal_uInt32          GetNumOfEntries() const { return m_aPicklistVector.size(); }
        void                CreateMenuEntries( Menu* pMenu );
        void                ExecuteMenuEntry( USHORT nId );
        void                ExecuteEntry( sal_uInt32 nIndex );
        String              GetMenuEntryTitle( sal_uInt32 nIndex );

        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif // _SFX_PICKLIST_HXX_
