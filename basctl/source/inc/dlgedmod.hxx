/*************************************************************************
 *
 *  $RCSfile: dlgedmod.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tbe $ $Date: 2001-02-26 10:56:29 $
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

#ifndef _BASCTL_DLGEDMOD_HXX
#define _BASCTL_DLGEDMOD_HXX

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif


//============================================================================
// DlgEdModel
//============================================================================

class DlgPage;
class Window;
class SfxObjectShell;

class DlgEdModel : public SdrModel
{
    friend class DlgPage;

private:
    SfxObjectShell* pObjectShell;

    DlgEdModel( const DlgEdModel& );
    void operator=(const DlgEdModel& rSrcModel);
    FASTBOOL operator==(const DlgEdModel& rCmpModel) const;

public:
    TYPEINFO();

    DlgEdModel(SfxItemPool* pPool=NULL, SvPersist* pPers=NULL );
    DlgEdModel(const String& rPath, SfxItemPool* pPool=NULL,
                SvPersist* pPers=NULL );
    DlgEdModel(SfxItemPool* pPool, SvPersist* pPers, FASTBOOL bUseExtColorTable );
    DlgEdModel(const String& rPath, SfxItemPool* pPool, SvPersist* pPers,
                FASTBOOL bUseExtColorTable );
    virtual ~DlgEdModel();

    // ruft SetChanged auf!! Aufgabe: Der Writer kann nicht SetChanged
    // ueberladen, da diese Funktion auch beim Umformatieren
    // des Textes (veraendert im Writer-Sinne nicht das Model!)
    // gerufen wird, und deshalb an sehr vielen Stellen abgeklemmt
    // werden muesste.
    virtual void DlgEdModelChanged( FASTBOOL bChanged = TRUE );

    SfxObjectShell* GetObjectShell() const { return pObjectShell; }
    void SetObjectShell( SfxObjectShell* pShell ) { pObjectShell = pShell; }

    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);

    virtual Window* GetCurDocViewWin();
};

#endif

