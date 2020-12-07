/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_IPRIORITABLE_HXX
#define INCLUDED_VCL_IPRIORITABLE_HXX

#include <vcl/dllapi.h>

#define VCL_PRIORITY_DEFAULT -1

namespace vcl
{

class VCL_DLLPUBLIC IPrioritable
{
protected:
    IPrioritable() : m_nPriority(VCL_PRIORITY_DEFAULT)
    {
    }

public:
    virtual ~IPrioritable()
    {
    }

    int GetPriority() const
    {
        return m_nPriority;
    }

    void SetPriority(int nPriority)
    {
        m_nPriority = nPriority;
    }

    virtual void HideContent() = 0;
    virtual void ShowContent() = 0;
    virtual bool IsHidden() = 0;

private:
    int m_nPriority;
};

} // namespace vcl

#endif // INCLUDED_VCL_IPRIORITABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
