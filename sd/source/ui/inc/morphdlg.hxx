/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: morphdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:45:46 $
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

#ifndef SD_MORPH_DLG_HXX
#define SD_MORPH_DLG_HXX

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

class SdrObject;

namespace sd {

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class MorphDlg
    : public ModalDialog
{
public:
    MorphDlg (
        ::Window* pParent,
        const SdrObject* pObj1,
        const SdrObject* pObj2);
    virtual ~MorphDlg (void);

    void            SaveSettings() const;
    USHORT          GetFadeSteps() const { return (USHORT) aMtfSteps.GetValue(); }
    BOOL            IsAttributeFade() const { return aCbxAttributes.IsChecked(); }
    BOOL            IsOrientationFade() const { return aCbxOrientation.IsChecked(); }

private:
    FixedLine       aGrpPreset;
    FixedText       aFtSteps;
    MetricField     aMtfSteps;
    CheckBox        aCbxAttributes;
    CheckBox        aCbxOrientation;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    SdrObject*      pSdrObj1;
    SdrObject*      pSdrObj2;

    void            LoadSettings();
};


#endif


} // end of namespace sd
