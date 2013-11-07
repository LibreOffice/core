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



#ifndef ARY_ESTACK_HXX
#define ARY_ESTACK_HXX



// USED SERVICES
    // BASE CLASSES
#include <slist>
    // COMPONENTS
    // PARAMETERS



template <class ELEM>
class EStack : private std::slist<ELEM>
{
  private:
    typedef std::slist<ELEM>    base;
    const base &        Base() const            { return *this; }
    base &              Base()                  { return *this; }

  public:
    typedef ELEM                                    value_type;
    typedef typename std::slist<ELEM>::size_type    size_type;

    // LIFECYCLE
                        EStack()                {}
                        EStack(
                            const EStack &      i_rStack )
                                                :   base( (const base &)(i_rStack) ) {}
                        ~EStack()               {}
    // OPERATORS
    EStack &            operator=(
                            const EStack &      i_rStack )
                                                { base::operator=( i_rStack.Base() );
                                                  return *this; }
    bool                operator==(
                            const EStack<ELEM> &
                                                i_r2 ) const
                                                { return std::operator==( Base(), this->i_rStack.Base() ); }
    bool                operator<(
                            const EStack<ELEM> &
                                                i_r2 ) const
                                                { return std::operator<( Base(), this->i_rStack.Base() ); }
    // OPERATIONS
    void                push(
                            const value_type &  i_rElem )
                                                { base::push_front(i_rElem); }
    void                pop()                   { base::pop_front(); }
    void                erase_all()             { while (NOT empty()) pop(); }

    // INQUIRY
    const value_type &  top() const             { return base::front(); }
    bool                empty() const           { return base::empty(); }

    // ACCESS
    value_type &        top()                   { return base::front(); }
};



// IMPLEMENTATION


#endif

