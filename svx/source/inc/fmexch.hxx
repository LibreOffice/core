/*************************************************************************
 *
 *  $RCSfile: fmexch.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
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
#ifndef _SVX_FMEXCH_HXX
#define _SVX_FMEXCH_HXX

#include <unotools/stl_types.hxx>

#ifndef _SOT_DTRANS_HXX //autogen
#include <sot/dtrans.hxx>
#endif

#ifndef _DTRANS_HXX //autogen
#include <so3/dtrans.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

class FmFormShell;
class FmFormPage;
class SvLBoxEntry;

//========================================================================
// Exchange types
#define SVX_FM_FIELD_EXCH           String("SvxFormFieldExch",              sizeof("SvxFormFieldExch"))
#define SVX_FM_CONTROL_EXCH         String("SvxFormExplCtrlExch",           sizeof("SvxFormExplCtrlExch"))
#define SVX_FM_CONTROLS_AS_PATH     String("SvxFormControlsAsPathExchange", sizeof("SvxFormControlsAsPathExchange"))
#define SVX_FM_HIDDEN_CONTROLS      String("SvxFormHiddenControlsExchange", sizeof("SvxFormHiddenControlsExchange"))
#define SVX_FM_FILTER_FIELDS        String("SvxFilterFieldExchange",        sizeof("SvxFilterFieldExchange"))

//========================================================================

typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<sal_uInt32> > FmControlPaths;

//========================================================================
class SvTreeListBox;
class SvxFmExplCtrlExch : public SvDataObject
{
    FmFormShell*    m_pShell;
    FmFormPage*     m_pPage;

    SvDataTypeList  m_aDataTypeList;

    std::vector<SvLBoxEntry*>   m_aDraggedEntries;
    FmControlPaths          m_aControlPaths;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >    m_seqControls;

public:
    SvxFmExplCtrlExch( const std::vector<SvLBoxEntry*>& lstWhich, FmFormShell* pShell, FmFormPage* pPage );
    SvxFmExplCtrlExch( SvLBoxEntry* pEntry );

    virtual const SvDataTypeList& GetTypeList() const { return m_aDataTypeList; }
    virtual sal_Bool  GetData( SvData* );

    SvLBoxEntry* GetDragEntry() const { return m_aDraggedEntries.front(); }
    void AddItem(SvLBoxEntry* pNew) { m_aDraggedEntries.push_back( pNew ); }

    FmFormShell*                    GetShell() const            { return m_pShell; }
    FmFormPage*                     GetPage() const             { return m_pPage; }
    const ::std::vector<SvLBoxEntry*>&      GetDraggedEntries() const   { return m_aDraggedEntries; }
    const FmControlPaths            GetControlPaths()           { return m_aControlPaths; }
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >  GetHiddenControls()         { return m_seqControls; }

    void BuildPathFormat(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot);
        // baut aus m_aDraggedEntries m_aControlPaths auf
        // (es wird davon ausgegangen, dass die Eintraege in m_aDraggedEntries sortiert sind in Bezug auf die Nachbar-Beziehung)
    void BuildListFromPath(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot);
        // der umgekehrte Weg : wirft alles aus m_aDraggedEntries weg und baut es mittels m_aControlPaths neu auf

    void AddHiddenControlsFormat(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > seqInterfaces);
        // fuegt ein SVX_FML_HIDDEN_CONTROLS-Format hinzu und merk sich dafuer die uebergebenen Interfaces
        // (es erfolgt KEINE Ueberpruefung, ob dadurch auch tatsaechlich nur hidden Controls bezeichnet werden, dass muss der
        // Aufrufer sicherstellen)
};

SV_DECL_IMPL_REF( SvxFmExplCtrlExch );

//========================================================================
class SvxFmFieldExch : public SotDataObject
{
    String aFieldDesc;
    SvDataTypeList  aDataTypeList;

public:
    SvxFmFieldExch(const String& rFieldDesc);

    virtual const SvDataTypeList& GetTypeList() const { return aDataTypeList; }
    virtual sal_Bool  GetData( SvData* );
};

SV_DECL_IMPL_REF( SvxFmFieldExch );
#endif

