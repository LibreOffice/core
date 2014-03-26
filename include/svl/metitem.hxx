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
#ifndef INCLUDED_SVL_METITEM_HXX
#define INCLUDED_SVL_METITEM_HXX

#include <svl/svldllapi.h>
#include <svl/intitem.hxx>

DBG_NAMEEX_VISIBILITY(SfxMetricItem, SVL_DLLPUBLIC)



class SVL_DLLPUBLIC SfxMetricItem: public SfxInt32Item
{
public:
                             TYPEINFO_OVERRIDE();
    explicit                 SfxMetricItem( sal_uInt16 nWhich = 0, sal_uInt32 nValue = 0 );
                             SfxMetricItem( sal_uInt16 nWhich, SvStream & );
                             SfxMetricItem( const SfxMetricItem& );
                             ~SfxMetricItem() {
                                 DBG_DTOR(SfxMetricItem, 0); }

    virtual bool             ScaleMetrics( long lMult, long lDiv ) SAL_OVERRIDE;
    virtual bool             HasMetrics() const SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
