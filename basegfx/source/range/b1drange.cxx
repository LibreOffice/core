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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b1irange.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B1DRange::B1DRange( const B1IRange& rRange ) :
        maRange()
    {
        if( !rRange.isEmpty() )
        {
            maRange = rRange.getMinimum();
            expand(rRange.getMaximum());
        }
    }

    B1IRange fround(const B1DRange& rRange)
    {
        return rRange.isEmpty() ?
            B1IRange() :
            B1IRange( fround( rRange.getMinimum()),
                      fround( rRange.getMaximum()) );
    }

} // end of namespace basegfx

// eof
