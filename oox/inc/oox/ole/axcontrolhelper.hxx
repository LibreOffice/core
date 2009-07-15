/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axcontrolhelper.hxx,v $
 * $Revision: 1.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_OLE_AXCONTROLHELPER_HXX
#define OOX_OLE_AXCONTROLHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace graphic { class XGraphic; }
    namespace drawing { class XDrawPage; }
    namespace form { class XForm; }
} } }

namespace oox { namespace core { class FilterBase; } }

namespace oox {
namespace ole {

// ============================================================================

const sal_uInt32 AX_SYSCOLOR_WINDOWBACK     = 0x80000005;
const sal_uInt32 AX_SYSCOLOR_WINDOWFRAME    = 0x80000006;
const sal_uInt32 AX_SYSCOLOR_WINDOWTEXT     = 0x80000008;
const sal_uInt32 AX_SYSCOLOR_BUTTONFACE     = 0x8000000F;
const sal_uInt32 AX_SYSCOLOR_BUTTONTEXT     = 0x80000012;

// ============================================================================

enum AxDefaultColorMode
{
    AX_DEFAULTCOLORMODE_BGR,        /// OLE default color type is interpreted as BGR color.
    AX_DEFAULTCOLORMODE_PALETTE     /// OLE default color type is interpreted as palette color.
};

// ============================================================================

/** Helper functions and callbacks for ActiveX form control filters. */
class AxControlHelper
{
public:
    explicit            AxControlHelper(
                            const ::oox::core::FilterBase& rFilter,
                            AxDefaultColorMode eColorMode = AX_DEFAULTCOLORMODE_BGR );
    virtual             ~AxControlHelper();

    /** Returns the filter object that imports/exports the form controls. */
    inline const ::oox::core::FilterBase& getFilter() const { return mrFilter; }
    /** Returns the UNO form used to insert the control models. */
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >
                        getControlForm() const;
    /** Returns the UNO RGB color from the passed encoded OLE color. */
    sal_Int32           convertColor( sal_uInt32 nAxColor ) const;

protected:
    /** Derived classes returns the UNO form of the current context. Called exactly once. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >
                        createControlForm() const = 0;
    /** Derived classes may implement to resolve a palette index to an RGB color. */
    virtual sal_Int32   getPaletteColor( sal_uInt16 nPaletteIdx ) const;

private:
    const ::oox::core::FilterBase& mrFilter;
    const AxDefaultColorMode meColorMode;
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > mxForm;
    mutable bool        mbHasFormQuerried;
};

// ============================================================================

/** Helper functions and callbacks for ActiveX form controls embedded in a
    document. */
class AxEmbeddedControlHelper : public AxControlHelper
{
public:
    explicit            AxEmbeddedControlHelper(
                            const ::oox::core::FilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                            AxDefaultColorMode eColorMode = AX_DEFAULTCOLORMODE_BGR );

protected:
    /** Creates and returns the standard UNO form in the wrapped draw page. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >
                        createControlForm() const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxDrawPage;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

