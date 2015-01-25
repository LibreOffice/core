/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef BAR_CODE_RENDER_HXX
#define BAR_CODE_RENDER_HXX

class Rectangle;
class SdrObjGroup;
namespace rtl { class OUString; }

namespace svx {
  SdrObjGroup *BarCodeRender(const Rectangle &rLogicRect, const rtl::OUString &rData);
}

#endif // BAR_CODE_RENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
