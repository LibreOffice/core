/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impsvgdialog.hxx,v $
 *
 *  $Revision: 1.1.2.5 $
 *
 *  last change: $Author: ka $ $Date: 2006/03/24 10:57:24 $
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

#ifndef IMPSVGDIALOG_HXX
#define IMPSVGDIALOG_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/FilterConfigItem.hxx>

#define SVG_EXPORTFILTER_CONFIGPATH         "Office.Common/Filter/SVG/Export/"
#define SVG_PROP_TINYPROFILE                "TinyMode"
#define SVG_PROP_EMBEDFONTS                 "EmbedFonts"
#define SVG_PROP_NATIVEDECORATION           "UseNativeTextDecoration"
#define SVG_PROP_GLYPHPLACEMENT             "GlyphPlacement"
#define SVG_PROP_OPACITY                    "Opacity"
#define SVG_PROP_GRADIENT                   "Gradient"

// ----------------
// - ImpSVGDialog -
// ----------------

class ResMgr;
class Window;

class ImpSVGDialog : public ModalDialog
{
private:

    FixedLine           maFI;
    CheckBox            maCBTinyProfile;
    CheckBox            maCBEmbedFonts;
    CheckBox            maCBUseNativeDecoration;

    OKButton            maBTOK;
    CancelButton        maBTCancel;
    HelpButton          maBTHelp;

    FilterConfigItem    maConfigItem;
    sal_Bool            mbOldNativeDecoration;

    DECL_LINK( OnToggleCheckbox, CheckBox* );

public:

    ImpSVGDialog( Window* pParent, /*ResMgr& rResMgr,*/
                  com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rFilterData );
    ~ImpSVGDialog();

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetFilterData();
};

#endif // IMPSVGDIALOG_HXX
