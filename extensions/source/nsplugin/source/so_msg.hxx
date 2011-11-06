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



#ifndef __PLUGIN_MSG_HXX
#define __PLUGIN_MSG_HXX
enum msg_type
{
    SO_EMPTY,        // 0
    SO_SET_WINDOW,   // 1
    SO_SET_URL,      // 2
    SO_NEW_INSTANCE, // 3
    SO_DESTROY,      // 4
    SO_SHUTDOWN,     // 5
    SO_PRINT         // 6
};

#define plugin_Int32 unsigned long int

typedef struct _PLUGIN_MSG {
    plugin_Int32 msg_id;
    plugin_Int32 instance_id;
    plugin_Int32 wnd_id;
    plugin_Int32 wnd_x;
    plugin_Int32 wnd_y;
    plugin_Int32 wnd_w;
    plugin_Int32 wnd_h;
    char url[484];
} PLUGIN_MSG;

#endif
