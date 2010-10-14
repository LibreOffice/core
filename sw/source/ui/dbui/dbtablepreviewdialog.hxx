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
#ifndef _DBTABLEPREVIEWDIALOG_HXX
#define _DBTABLEPREVIEWDIALOG_HXX

#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/uno/Sequence.h>

namespace com{ namespace sun{ namespace star{
    namespace beans{  struct PropertyValue; }
    namespace frame{ class XFrame;     }
    }}}


/*-- 08.04.2004 14:04:29---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwDBTablePreviewDialog : public SfxModalDialog
{
    FixedInfo       m_aDescriptionFI;
    Window*         m_pBeamerWIN;
    OKButton        m_aOK;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >         m_xFrame;
public:
    SwDBTablePreviewDialog(Window* pParent,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rValues  );
    ~SwDBTablePreviewDialog();

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
