/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <dispatch/dispatchdisabler.hxx>
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
                        IFFACTORY( ::framework::MediaTypeDetectionHelper                )
                        IFFACTORY( ::framework::MailToDispatcher                        )   else
                        IFFACTORY( ::framework::ServiceHandler                          )   else
                        IFFACTORY( ::framework::LogoTextStatusbarController             )   else
                        IFFACTORY( ::framework::LogoImageStatusbarController            )   else
                        IFFACTORY( ::framework::License                                 )   else
                        IFFACTORY( ::framework::PopupMenuDispatcher                     )   else
                        IFFACTORY( ::framework::DispatchHelper                          )   else
                        IFFACTORY( ::framework::DispatchDisabler                        )   else
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
