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

#ifndef SD_GLUECTRL_HXX
#define SD_GLUECTRL_HXX

#include <vcl/lstbox.hxx>
#include <sfx2/tbxctrl.hxx>

/*************************************************************************
|*
|* GluePointEscDirLB
|*
\************************************************************************/
class GlueEscDirLB : public ListBox
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
public:
                GlueEscDirLB( Window* pParent,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                 ~GlueEscDirLB();

    virtual void Select();

    void         Fill();
};

/*************************************************************************
|*
|* Toolbox-Controller fuer Klebepunkte-Austrittsrichtung
|*
\************************************************************************/

class SdTbxCtlGlueEscDir: public SfxToolBoxControl
{
private:
    sal_uInt16  GetEscDirPos( sal_uInt16 nEscDir );

public:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlGlueEscDir( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
            ~SdTbxCtlGlueEscDir() {}
};

#endif // SD_GLUECTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
