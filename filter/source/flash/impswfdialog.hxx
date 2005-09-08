/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impswfdialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:41:46 $
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

#ifndef IMPSWFDIALOG_HXX
#define IMPSWFDIALOG_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

#include <svtools/FilterConfigItem.hxx>

// ----------------
// - ImpSWFDialog -
// ----------------

class ResMgr;
class Window;

class ImpSWFDialog : public ModalDialog
{
private:
    FixedInfo           maFiDescr;
    NumericField        maNumFldQuality;
    FixedInfo           maFiExportAllDescr;
    CheckBox            maCheckExportAll;
    FixedInfo           maFiExportBackgroundsDescr;
    CheckBox            maCheckExportBackgrounds;
    FixedInfo           maFiExportBackgroundObjectsDescr;
    CheckBox            maCheckExportBackgroundObjects;
    FixedInfo           maFiExportSlideContentsDescr;
    CheckBox            maCheckExportSlideContents;
    FixedInfo           maFiExportSoundDescr;
    CheckBox            maCheckExportSound;
    FixedInfo           maFiExportOLEAsJPEGDescr;
    CheckBox            maCheckExportOLEAsJPEG;
    FixedInfo           maFiExportMultipleFilesDescr;
    CheckBox            maCheckExportMultipleFiles;

    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    FilterConfigItem    maConfigItem;

    DECL_LINK( OnToggleCheckbox, CheckBox* );

public:
    ImpSWFDialog( Window* pParent, ResMgr& rResMgr,
                  com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rFilterData );
    ~ImpSWFDialog();

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetFilterData();
};

#endif // IMPDIALOG_HXX
