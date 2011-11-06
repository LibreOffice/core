/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef LAYOUT_CORE_DIALOGBUTTONHBOX_HXX
#define LAYOUT_CORE_DIALOGBUTTONHBOX_HXX

#include <layout/core/box.hxx>
#include <layout/core/flow.hxx>

namespace layoutimpl
{

class DialogButtonHBox : public HBox
{
public:
    DialogButtonHBox();

    void setOrdering( rtl::OUString const& ordering );
    void SAL_CALL addChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild ) throw ( css::uno::RuntimeException, css::awt::MaxChildrenException );
    void SAL_CALL removeChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild ) throw ( css::uno::RuntimeException );

private:
    enum Ordering { PLATFORM, GNOME, KDE, MACOS, WINDOWS };

    void orderChildren();
    void gnomeOrdering();
    void kdeOrdering();
    void macosOrdering();
    void windowsOrdering();

    static Ordering const DEFAULT_ORDERING;
    Ordering mnOrdering;
    Flow mFlow;

    ChildData *mpAction; /* [..]?, [Retry?] */
    ChildData *mpAffirmative; /* OK, Yes, Save */
    ChildData *mpAlternate; /* NO, [Ignore?], Don't save, Quit without saving */
    ChildData *mpApply; /* Deprecated? */
    ChildData *mpCancel; /* Cancel, Close */
    ChildData *mpFlow;
    ChildData *mpHelp;
    ChildData *mpReset;

    std::list< Box_Base::ChildData *> maOther;
};

} // namespace layoutimpl

#endif /* LAYOUT_CORE_DIALOGBUTTONHBOX_HXX */
