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

#include <macros/registration.hxx>

/*=================================================================================================================
    Add new include and new register info to for new services.

    Example:

        #include <service1.hxx>
        #include <service2.hxx>

        COMPONENTGETFACTORY (   IFFACTORIE( Service1 )
                                 else
                                IFFACTORIE( Service2 )
                             )
=================================================================================================================*/
#include <services/mediatypedetectionhelper.hxx>
#include <dispatch/mailtodispatcher.hxx>
#include <dispatch/oxt_handler.hxx>
#include <dispatch/popupmenudispatcher.hxx>
#include <dispatch/servicehandler.hxx>
#include <services/dispatchhelper.hxx>
#include <recording/dispatchrecorder.hxx>
#include <recording/dispatchrecordersupplier.hxx>
#include <services/license.hxx>
#include <services/uriabbreviation.hxx>
#include <uielement/simpletextstatusbarcontroller.hxx>
#include <uielement/logoimagestatusbarcontroller.hxx>
#include <uielement/logotextstatusbarcontroller.hxx>
#include <uielement/fontmenucontroller.hxx>
#include <uielement/fontsizemenucontroller.hxx>
#include <uielement/footermenucontroller.hxx>
#include <uielement/headermenucontroller.hxx>
#include <uielement/langselectionmenucontroller.hxx>
#include <uielement/macrosmenucontroller.hxx>
#include <uielement/newmenucontroller.hxx>
#include <uielement/toolbarsmenucontroller.hxx>
#include <uielement/popupmenucontroller.hxx>

COMPONENTGETFACTORY ( fwl,
                        IFFACTORY( ::framework::MediaTypeDetectionHelper        )
                        IFFACTORY( ::framework::MailToDispatcher                        ) else
                        IFFACTORY( ::framework::ServiceHandler                          )   else
                        IFFACTORY( ::framework::LogoTextStatusbarController             )   else
                        IFFACTORY( ::framework::LogoImageStatusbarController            )   else
                        IFFACTORY( ::framework::License                                 )   else
                        IFFACTORY( ::framework::PopupMenuDispatcher                     )   else
                        IFFACTORY( ::framework::DispatchHelper                          )   else
                        IFFACTORY( ::framework::DispatchRecorder                        )   else
                        IFFACTORY( ::framework::DispatchRecorderSupplier                )   else
                        IFFACTORY( ::framework::SimpleTextStatusbarController           )   else
                        IFFACTORY( ::framework::ToolbarsMenuController                  )   else
                        IFFACTORY( ::framework::FontMenuController                      )   else
                        IFFACTORY( ::framework::MacrosMenuController                    )   else
                        IFFACTORY( ::framework::NewMenuController                       )   else
                        IFFACTORY( ::framework::FontSizeMenuController                  )   else
                        IFFACTORY( ::framework::UriAbbreviation                         )   else
                        IFFACTORY( ::framework::FooterMenuController                    )   else
                        IFFACTORY( ::framework::HeaderMenuController                    )   else
                        IFFACTORY( ::framework::LanguageSelectionMenuController         )   else
                        IFFACTORY( ::framework::Oxt_Handler                             )   else
                        IFFACTORY( ::framework::PopupMenuController                     )
                    )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
