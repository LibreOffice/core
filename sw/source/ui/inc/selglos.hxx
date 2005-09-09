/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selglos.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:00:40 $
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
#ifndef _SELGLOS_HXX
#define _SELGLOS_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwSelGlossaryDlg : public ModalDialog
{
protected:
    ListBox         aGlosBox;
    FixedLine       aGlosFL;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    DECL_LINK(DoubleClickHdl, ListBox*);
public:
    SwSelGlossaryDlg(Window * pParent, const String &rShortName);
    ~SwSelGlossaryDlg();
    void InsertGlos(const String &rRegion, const String &rGlosName);    // inline
    USHORT GetSelectedIdx() const;  // inline
    void SelectEntryPos(USHORT nIdx);   // inline
};

inline void SwSelGlossaryDlg::InsertGlos(const String &rRegion,
                                const String &rGlosName)
{
    String aTmp( rRegion );
    aTmp += ':';
    aTmp += rGlosName;
    aGlosBox.InsertEntry( aTmp );
}
inline USHORT SwSelGlossaryDlg::GetSelectedIdx() const
{   return aGlosBox.GetSelectEntryPos(); }
inline void SwSelGlossaryDlg::SelectEntryPos(USHORT nIdx)
{   aGlosBox.SelectEntryPos(nIdx); }

#endif
