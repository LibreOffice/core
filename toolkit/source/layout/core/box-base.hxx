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



#ifndef LAYOUT_CORE_BOX_BASE_HXX
#define LAYOUT_CORE_BOX_BASE_HXX

#include <layout/core/container.hxx>

#include <list>

namespace layoutimpl
{

class Box_Base : public Container
{
public:
    // Children properties
    struct ChildData
    {
        css::uno::Reference< css::awt::XLayoutConstrains > mxChild;
        css::uno::Reference< css::beans::XPropertySet > mxProps;
        css::awt::Size maRequisition;
        virtual bool isVisible();

        ChildData( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );
        virtual ~ChildData() { };
    };

    struct ChildProps: public PropHelper
    {
        //ChildProps( ChildProps* );
    };

protected:
    std::list< ChildData* > maChildren;


    virtual ChildData *createChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild ) = 0;
    virtual ChildProps *createChildProps( ChildData* pData ) = 0;

    ChildData *removeChildData( std::list< ChildData *>&, css::uno::Reference< css::awt::XLayoutConstrains > const& Child );

public:
    void AddChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child);

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child)
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Reference
                                < css::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);
};

} //  namespace layoutimpl

#endif /* LAYOUT_CORE_BOX_BASE HXX */
