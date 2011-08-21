/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _OOXMLEXPORT_HXX
#define _OOXMLEXPORT_HXX

#include <sax/fshelper.hxx>
#include <oox/core/filterbase.hxx>
#include "filter/msfilter/msfilterdllapi.h"

/**
 Interface class, StarMath will implement writeFormulaOoxml() to write out OOXML
 representing the formula.
 */
class MSFILTER_DLLPUBLIC OoxmlFormulaExportBase
{
public:
    virtual void writeFormulaOoxml( ::sax_fastparser::FSHelperPtr m_pSerializer, oox::core::OoxmlVersion version ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
