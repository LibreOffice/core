/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVX_FMEXCH_HXX
#define _SVX_FMEXCH_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _TRANSFER_HXX
#include <bf_svtools/transfer.hxx>
#endif

class SvTreeListBox;
namespace binfilter {

class FmFormShell;
class FmFormPage;
class SvLBoxEntry;

//========================================================================
// Exchange types
#define SVX_FM_FIELD_EXCH			String("SvxFormFieldExch",				sizeof("SvxFormFieldExch"))
#define SVX_FM_CONTROL_EXCH			String("SvxFormExplCtrlExch",			sizeof("SvxFormExplCtrlExch"))
#define SVX_FM_CONTROLS_AS_PATH		String("SvxFormControlsAsPathExchange",	sizeof("SvxFormControlsAsPathExchange"))
#define SVX_FM_HIDDEN_CONTROLS		String("SvxFormHiddenControlsExchange",	sizeof("SvxFormHiddenControlsExchange"))
#define SVX_FM_FILTER_FIELDS		String("SvxFilterFieldExchange",		sizeof("SvxFilterFieldExchange"))

//========================================================================

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================

    DECLARE_STL_VECTOR( SvLBoxEntry*, ListBoxEntryArray );

    //====================================================================
    //= OLocalExchange
    //====================================================================

    //====================================================================
    //= OLocalExchangeHelper
    //====================================================================
    /// a helper for navigator windows (SvTreeListBox'es) which allow DnD within themself
    class OLocalExchangeHelper
    {
    protected:
        Window*				m_pDragSource;

    public:
        OLocalExchangeHelper( Window* _pDragSource );
        ~OLocalExchangeHelper();

        void		prepareDrag( );

        void		startDrag( sal_Int8 nDragSourceActions );
        void		copyToClipboard( ) const;



    protected:

    protected:
        void implReset();
    };

    //====================================================================
    //= OControlTransferData
    //====================================================================

    //====================================================================

    //====================================================================
    //= OControlExchange
    //====================================================================

    //====================================================================
    //= OControlExchangeHelper
    //====================================================================
/*N*/ 	class OControlExchangeHelper : public OLocalExchangeHelper
/*N*/ 	{
/*N*/ 	public:
/*N*/ 		OControlExchangeHelper(Window* _pDragSource) : OLocalExchangeHelper(_pDragSource) { }
/*N*/ 
/*N*/ 
/*N*/ 	protected:
/*N*/ 	};

    //====================================================================
    //====================================================================

//........................................................................
}	// namespace svxform
//........................................................................

}//end of namespace binfilter
#endif

