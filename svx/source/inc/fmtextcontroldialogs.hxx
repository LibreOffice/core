/*************************************************************************
 *
 *  $RCSfile: fmtextcontroldialogs.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 15:50:07 $
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

#ifndef SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX
#define SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX

#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= TextControlCharAttribDialog
    //====================================================================
    class TextControlCharAttribDialog : public SfxTabDialog
    {
    private:
        SvxFontListItem m_aFontList;

    public:
        TextControlCharAttribDialog( Window* pParent, const SfxItemSet& _rCoreSet, const SvxFontListItem& _rFontList );
        ~TextControlCharAttribDialog();

    protected:
        virtual void PageCreated( USHORT _nId, SfxTabPage& _rPage );
    };

    //====================================================================
    //= TextControlParaAttribDialog
    //====================================================================
    class TextControlParaAttribDialog : public SfxTabDialog
    {
    public:
        TextControlParaAttribDialog( Window* _pParent, const SfxItemSet& _rCoreSet );
        ~TextControlParaAttribDialog();

    protected:
        virtual void PageCreated( USHORT _nId, SfxTabPage& _rPage );
    };
//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX

