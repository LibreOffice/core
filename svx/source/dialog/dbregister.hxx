/*************************************************************************
 *
 *  $RCSfile: dbregister.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:37:25 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVX_DBREGISTER_HXX
#define SVX_DBREGISTER_HXX

#ifndef _SVX_OPTPATH_HXX
#include "optpath.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

class SvLBoxEntry;
namespace svx
{
    class OptHeaderTabListBox;
}
//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DbRegistrationOptionsPage
    //====================================================================
    class DbRegistrationOptionsPage : public SfxTabPage
    {
    private:
        FixedText           aTypeText;
        FixedText           aPathText;
        Control             aPathCtrl;
        PushButton          m_aNew;
        PushButton          m_aEdit;
        PushButton          m_aDelete;
        FixedLine           aStdBox;

        HeaderBar*          pHeaderBar;
        ::svx::OptHeaderTabListBox* pPathBox;
        SvLBoxEntry*        m_pCurEntry;
        ULONG               m_nOldCount;
        BOOL                m_bModified;


#ifdef SVX_DBREGISTER_HXX
        DECL_LINK( NewHdl, void * );
        DECL_LINK( EditHdl, void * );
        DECL_LINK( DeleteHdl, void * );

        DECL_LINK( PathSelect_Impl, SvTabListBox* );

        DECL_LINK( HeaderSelect_Impl, HeaderBar * );
        DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );
        DECL_LINK( NameValidator, String*);


        /** inserts a new entry in the tablistbox
            @param  _sName
                The name of the entry.
            @param  _sLocation
                The location of the file.
        */
        void insertNewEntry(const ::rtl::OUString& _sName,const ::rtl::OUString& _sLocation);

        /** opens the LinkDialog to create a register pair
            @param  _sOldName
                The old name of the entry may be empty.
            @param  _sOldLocation
                The old location of the entry may be empty.
            @param  _pEntry
                The entry to remove if the entry will be changed
        */
        void openLinkDialog(const String& _sOldName,const String& _sOldLocation,SvLBoxEntry* _pEntry = NULL);

        /** opens a file pciker to select a database file
            @param  _sLocation
                If set, the file picker use it as default directory
            @return
                the location of the database file
        */
        String getFileLocation(const String& _sLocation);
#endif

    public:
        DbRegistrationOptionsPage( Window* pParent, const SfxItemSet& rSet );
        virtual ~DbRegistrationOptionsPage();

        static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
        static USHORT*      GetRanges();

        virtual BOOL        FillItemSet( SfxItemSet& rSet );
        virtual void        Reset( const SfxItemSet& rSet );
        virtual void        FillUserData();
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_DBREGISTER_HXX


