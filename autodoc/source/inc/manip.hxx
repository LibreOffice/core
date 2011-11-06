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



#ifndef ARY_MANIP_HXX
#define ARY_MANIP_HXX

template <class XY >
class Manipulator
{
  public:
    virtual             ~Manipulator() {}

    void                operator()(
                            XY &                io_r ) const
                            { op_fcall(io_r); }
  private:
    virtual void        op_fcall(
                            XY &                io_r ) const = 0;
};

template <class XY >
class Const_Manipulator
{
  public:
    virtual             ~Const_Manipulator() {}

    void                operator()(
                            const XY &          io_r ) const
                            { op_fcall(io_r); }
  private:
    virtual void        op_fcall(
                            const XY &          io_r ) const = 0;
};


#endif

