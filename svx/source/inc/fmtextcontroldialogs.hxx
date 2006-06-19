/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtextcontroldialogs.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 16:07:06 $
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
#include <flstitem.hxx>
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
    };
//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_SOURCE_FORM_FMTEXTCONTROLDIALOGS_HXX

