/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SFX_INETTBC_HXX
#define _SFX_INETTBC_HXX

// includes *****************************************************************
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

#include <vcl/combobox.hxx>
#include <svtools/acceleratorexecute.hxx>

#if _SOLAR__PRIVATE

#include <sfx2/tbxctrl.hxx>
class SvtURLBox;

class SfxURLToolBoxControl_Impl : public SfxToolBoxControl
{
private:
    ::svt::AcceleratorExecute*  pAccExec;

    SvtURLBox*              GetURLBox() const;
    void                    OpenURL( const String& rName, sal_Bool bNew ) const;

    DECL_LINK(              OpenHdl, void* );
    DECL_LINK(              SelectHdl, void* );
    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    struct ExecuteInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
        ::com::sun::star::util::URL                                                aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
    };

    DECL_STATIC_LINK( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo* );

public:

                            SFX_DECL_TOOLBOX_CONTROL();

                            SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    virtual                 ~SfxURLToolBoxControl_Impl();

    virtual Window*         CreateItemWindow( Window* pParent );
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
};

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
