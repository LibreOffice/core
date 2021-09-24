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

#pragma once

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <o3tl/cow_wrapper.hxx>

class SvStream;
class ImplJobSetup;

class VCL_DLLPUBLIC JobSetup
{
    friend class Printer;

public:
    JobSetup();
    JobSetup( const JobSetup& rJob );
    ~JobSetup();

    JobSetup&           operator=( const JobSetup& rJob );
    JobSetup&           operator=( JobSetup&& rJob );

    bool                operator==( const JobSetup& rJobSetup ) const;
    bool                operator!=( const JobSetup& rJobSetup ) const
                            { return !(JobSetup::operator==( rJobSetup )); }

    SAL_DLLPRIVATE ImplJobSetup&        ImplGetData();
    SAL_DLLPRIVATE const ImplJobSetup&  ImplGetConstData() const;

    OUString const &      GetPrinterName() const;
    bool                  IsDefault() const;

    friend VCL_DLLPUBLIC SvStream&  ReadJobSetup( SvStream& rIStream, JobSetup& rJobSetup );
    friend VCL_DLLPUBLIC SvStream&  WriteJobSetup( SvStream& rOStream, const JobSetup& rJobSetup );

    typedef o3tl::cow_wrapper< ImplJobSetup > ImplType;

private:
    ImplType        mpImpl;
};

namespace std
{
    /** Specialise std::optional template for the case where we are wrapping a o3tl::cow_wrapper
        type, and we can make the pointer inside the cow_wrapper act as an empty value,
        and save ourselves some storage */
    template<>
    class VCL_DLLPUBLIC optional<JobSetup> final : public o3tl::cow_optional<JobSetup>
    {
    public:
        using cow_optional::cow_optional; // inherit constructors
        optional(const optional&) = default;
        optional(optional&&) = default;
        optional& operator=(const optional&) = default;
        optional& operator=(optional&&) = default;
        ~optional();
        void reset();
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
