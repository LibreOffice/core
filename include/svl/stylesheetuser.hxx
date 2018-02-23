/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_STYLESHEETUSER_HXX
#define INCLUDED_SVL_STYLESHEETUSER_HXX

#include <sal/config.h>

#include <sal/types.h>

namespace svl
{

/** Test whether object that uses a stylesheet is used itself.

  This interface should be implemented by all classes that use
  a SfxStyleSheet (and listen on it). It can be queried by the stylesheet
  to determine if it is really used.
  */
class SAL_DLLPUBLIC_RTTI StyleSheetUser
{
public:
    /** Test whether this object is used.

      @return true, if the object is used, false otherwise
      */
    virtual bool isUsedByModel() const = 0;
protected:
    ~StyleSheetUser() {}
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
