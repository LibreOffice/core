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

#ifndef INCLUDED_ANIMATIONS_SOURCE_ANIMCORE_FACTREG_HXX
#define INCLUDED_ANIMATIONS_SOURCE_ANIMCORE_FACTREG_HXX

namespace animcore {

extern OUString getImplementationName_PAR();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_PAR();

extern OUString getImplementationName_SEQ();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_SEQ();

extern OUString getImplementationName_ITERATE();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_ITERATE();

extern OUString getImplementationName_ANIMATE();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_ANIMATE();

extern OUString getImplementationName_SET();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_SET();

extern OUString getImplementationName_ANIMATECOLOR();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_ANIMATECOLOR();

extern OUString getImplementationName_ANIMATEMOTION();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_ANIMATEMOTION();

extern OUString getImplementationName_ANIMATETRANSFORM();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_ANIMATETRANSFORM();

extern OUString getImplementationName_TRANSITIONFILTER();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_TRANSITIONFILTER();

extern OUString getImplementationName_AUDIO();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_AUDIO();

extern OUString getImplementationName_COMMAND();
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_COMMAND();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
