/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxpicklist.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:11:02 $
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

#ifndef _COM_SUN_STAR_UTIL_XSTRINGWIDTH_HPP_
#include <com/sun/star/util/XStringWidth.hpp>
#endif

#include <vector>

#define PICKLIST_MAXSIZE  100

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
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringWidth > m_xStringLength;

                            SfxPickList( ULONG nMenuSize );
                            ~SfxPickList();

    static osl::Mutex*      GetOrCreateMutex();

    void                    CreatePicklistMenuTitle( Menu* pMenu, USHORT nItemId, const String& aURL, sal_uInt32 nNo );
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
