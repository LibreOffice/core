/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_ICONTEXT_HXX
#define INCLUDED_VCL_ICONTEXT_HXX

#include <vcl/EnumContext.hxx>
#include <vector>

namespace vcl
{

class IContext
{
protected:
    IContext()
    {
        maContext.push_back( vcl::EnumContext::Context::Any );
    }

public:
    void SetContext(const std::vector<vcl::EnumContext::Context>& aContext)
    {
        maContext = aContext;
    }

    bool HasContext( const vcl::EnumContext::Context eContext ) const
    {
        auto aFind = std::find(maContext.begin(), maContext.end(), eContext);
        if (aFind == maContext.end())
            return false;
        return true;
    }

private:
    std::vector<vcl::EnumContext::Context> maContext;
};

} // namespace vcl

#endif // INCLUDED_VCL_ICONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
