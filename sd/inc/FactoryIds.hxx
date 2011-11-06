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



#ifndef SD_FACTORY_IDS_HXX
#define SD_FACTORY_IDS_HXX

namespace sd {

/** This are ids used by SfxTopFrame::Create() or CreateViewFrame() to
    select a factory to create an instance of an SfxViewShell super
    class.  This allows the caller to create a view shell directly
    with a certain view instead of first create a default view and
    then switch to the desired view.
*/
enum ViewShellFactoryIds
{
    IMPRESS_FACTORY_ID = 1,
    DRAW_FACTORY_ID = 1,
    SLIDE_SORTER_FACTORY_ID = 2,
    OUTLINE_FACTORY_ID = 3,
    PRESENTATION_FACTORY_ID = 4
};

} // end of namespace sd

#endif

