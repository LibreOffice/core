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



#if ! defined INCLUDED_DP_GUI_H
#define INCLUDED_DP_GUI_H

#include "dp_gui_updatedata.hxx"
#include "dp_misc.h"
#include "dp_gui.hrc"
#include "rtl/ref.hxx"
#include "rtl/instance.hxx"
#include "osl/thread.hxx"
#include "cppuhelper/implbase2.hxx"
#include "vcl/svapp.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include "svtools/svtabbx.hxx"
#include "svtools/headbar.hxx"
#include "com/sun/star/ucb/XContentEventListener.hpp"
#include "osl/mutex.hxx"
#include <list>
#include <memory>
#include <queue>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
    }
    namespace frame {
        class XDesktop;
    }
    namespace awt {
        class XWindow;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace deployment {
        class XPackageManagerFactory;
    }
} } }

namespace svt {
    class FixedHyperlink;
}

namespace dp_gui {

enum PackageState { REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE };

//==============================================================================

class SelectedPackage: public salhelper::SimpleReferenceObject {
public:
    SelectedPackage() {}
    SelectedPackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> &xPackage)
    : m_xPackage( xPackage )
    {}

    virtual ~SelectedPackage();
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> getPackage() const { return m_xPackage; }

private:
    SelectedPackage(SelectedPackage &); // not defined
    void operator =(SelectedPackage &); // not defined

    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> m_xPackage;
};

} // namespace dp_gui

#endif
