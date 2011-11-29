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



namespace testtools { namespace bridgetest { namespace cli_cs {

public class Multi: unoidl.test.testtools.bridgetest.XMulti
{
    public Multi()
    {
    }

    public double att1
    {
        get { return _att1; }
        set { _att1 = value; }
    }

    public int fn11(int arg)
    {
        return 11 * arg;
    }

    public string fn12(string arg)
    {
        return "12" + arg;
    }

    public int fn21(int arg)
    {
        return 21 * arg;
    }

    public string fn22(string arg)
    {
        return "22" + arg;
    }

    public double att3
    {
        get { return _att3; }
        set { _att3 = value; }
    }

    public int fn31(int arg)
    {
        return 31 * arg;
    }

    public string fn32(string arg)
    {
        return "32" + arg;
    }

    public int fn33()
    {
        return 33;
    }

    public int fn41(int arg)
    {
        return 41 * arg;
    }

    public int fn61(int arg)
    {
        return 61 * arg;
    }

    public string fn62(string arg)
    {
        return "62" + arg;
    }

    public int fn71(int arg)
    {
        return 71 * arg;
    }

    public string fn72(string arg)
    {
        return "72" + arg;
    }

    public int fn73()
    {
        return 73;
    }

    private double _att1;
    private double _att3;
};

} } }


