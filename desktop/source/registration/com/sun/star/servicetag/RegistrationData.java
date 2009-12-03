
/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 1997-2008 Sun Microsystems, Inc. All rights reserved.
 *
 * The contents of this file are subject to the terms of either the GNU
 * General Public License Version 2 only ("GPL") or the Common Development
 * and Distribution License("CDDL") (collectively, the "License").  You
 * may not use this file except in compliance with the License. You can obtain
 * a copy of the License at https://glassfish.dev.java.net/public/CDDL+GPL.html
 * or glassfish/bootstrap/legal/LICENSE.txt.  See the License for the specific
 * language governing permissions and limitations under the License.
 *
 * When distributing the software, include this License Header Notice in each
 * file and include the License file at glassfish/bootstrap/legal/LICENSE.txt.
 * Sun designates this particular file as subject to the "Classpath" exception
 * as provided by Sun in the GPL Version 2 section of the License file that
 * accompanied this code.  If applicable, add the following below the License
 * Header, with the fields enclosed by brackets [] replaced by your own
 * identifying information: "Portions Copyrighted [year]
 * [name of copyright owner]"
 *
 * Contributor(s):
 *
 * If you wish your version of this file to be governed by only the CDDL or
 * only the GPL Version 2, indicate your decision by adding "[Contributor]
 * elects to include this software in this distribution under the [CDDL or GPL
 * Version 2] license."  If you don't indicate a single choice of license, a
 * recipient has the option to distribute your version of this file under
 * either the CDDL, the GPL Version 2 or to extend the choice of license to
 * its licensees as provided above.  However, if you add GPL Version 2 code
 * and therefore, elected the GPL Version 2 license, then the option applies
 * only if the new code is made subject to such option by the copyright
 * holder.
 */

package com.sun.star.servicetag;

import java.io.*;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import static com.sun.star.servicetag.RegistrationDocument.*;

/**
 * A {@code RegistrationData} object is a container of one or more 
 * {@link #getServiceTags service tags} that identify the 
 * components for product registration.
 * Each {@code RegistrationData} object has a {@link #getRegistrationURN
 * uniform resource name} (URN) as its identifier.
 * <a name="EnvMap"></a>
 * It also has an <i>environment map</i> with
 * the following elements:
 * <blockquote>
 *       <table border=0>
 *       <tr>
 *          <td><tt>hostname</tt></td>
 *          <td>Hostname of the system</td>
 *          <td>e.g. woody</td>
 *       </tr>
 *       <tr>
 *          <td><tt>hostId</tt></td>
 *          <td>Host ID of the system</td>
 *          <td>e.g. 83abc1ab</td>
 *       </tr>
 *       <tr>
 *          <td><tt>osName</tt></td>
 *          <td>Operating system name</td>
 *          <td> e.g. SunOS</td>
 *       </tr>
 *       <tr>
 *          <td><tt>osVersion</tt></td>
 *          <td>Operating system version</td>
 *          <td> e.g. 5.10</td>
 *       </tr>
 *       <tr>
 *          <td><tt>osArchitecture</tt></td>
 *          <td>Operating system architecture</td>
 *          <td> e.g. sparc</td>
 *       </tr>
 *       <tr>
 *          <td><tt>systemModel</tt></td>
 *          <td>System model</td>
 *          <td> e.g. SUNW,Sun-Fire-V440</td>
 *       </tr>
 *       <tr>
 *          <td><tt>systemManufacturer</tt></td>
 *          <td>System manufacturer</td>
 *          <td> e.g. Sun Microsystems</td>
 *       </tr>
 *       <tr>
 *          <td><tt>cpuManufacturer</tt></td>
 *          <td>CPU manufacturer</td>
 *          <td> e.g. Sun Microsystems</td>
 *       </tr>
 *       <tr>
 *          <td><tt>serialNumber</tt></td>
 *          <td>System serial number</td>
 *          <td> e.g. BEL078932</td>
 *       </tr>
 *       <tr>
 *          <td><tt>physmem</tt></td>
 *          <td>Physical memory for the system (in MB)</td>
 *          <td> e.g. 4096</td>
 *       </tr>
 *       </table>
 *  </blockquote>
 * The <tt>hostname</tt> and <tt>osName</tt> element must have a non-empty value.
 * If an element is not available on a system and their value will be
 * empty.
 * <p>
 * <a name="XMLSchema">
 * <b>Registration XML Schema</b></a>
 * <p>
 * A {@code RegistrationData} object can be {@link #loadFromXML loaded} from 
 * and {@link #storeToXML stored} into an XML file in the format described 
 * by the
 * <a href="https://sn-tools.central.sun.com/twiki/pub/ServiceTags/RegistrationRelayService/product_registration.xsd">
 * registration data schema</a>.  The registration data schema is defined by the 
 * Service Tags Technology.
 * <p>
 * Typically the registration data is constructed at installation time
 * and stored in an XML file for later service tag lookup or registration.
 * 
 * <p>
 * <b>Example Usage</b>
 * <p>
 * The examples below show how the {@code RegistrationData} can be 
 * used for product registration.
 * Exception handling is not shown in these examples for clarity.
 * <ol>
 * <li>This example shows how the JDK creates a JDK service tag, installs it
 * in the system service tag registry and adds it to the registration data.
 * <br>
 * <blockquote><pre>
 *   // create a service tag object with an instance_urn
 *   ServiceTag st = ServiceTag.newInstance(ServiceTag.generateInstanceURN(),
 *                                          ....);
 *   // Adds to the system service tag registry if supported
 *   if (Registry.isSupported()) {
 *       Registry.getSystemRegistry().addServiceTag(st);
 *   }
 * 
 *   // add to the registration data
 *   RegistrationData registration  = new RegistrationData();
 *   registration.addServiceTag(st); 
 * </pre></blockquote>
 * </li>
 * <li>At this point, the registration data is ready to 
 * send to Sun Connection for registration. This example shows how to register 
 * the JDK via the <i>Registration Relay Service</i>.
 * <p>
 * There are several registration services for Sun Connection. For example,
 * the <a href="https://sn-tools.central.sun.com/twiki/bin/view/ServiceTags/RegistrationRelayService">
 * Registration Relay Service</a> is a web application interface that
 * processes the registration data payload sent via HTTP post
 * and hosts the registration user interface for a specified 
 * registration URL.  Refer to the 
 * Registration Relay Service Specification for details.
 * <p>
 * <blockquote><pre>
 *   // Open the connection to the URL of the registration service
 *   HttpsURLConnection con = (HttpsURLConnection) url.openConnection();
 *   con.setDoInput(true);
 *   con.setDoOutput(true);
 *   con.setUseCaches(false);
 *   con.setAllowUserInteraction(false);
 *   con.setRequestMethod("POST");
 *   con.setRequestProperty("Content-Type", "text/xml;charset=\"utf-8\"");
 *   con.connect();
 *
 *   // send the registration data to the registration service
 *   OutputStream out = con.getOutputStream();
 *   registration.storeToXML(out);
 *   out.close();
 * </pre></blockquote>
 * </li>
 * <li>This example shows how to store the registration data in an XML file.
 *     for later service tag lookup or registration.
 * <br>
 * <blockquote><pre>
 *   BufferedOutputStream out = new BufferedOutputStream(
 *       new FileOutputStream(""&lt;JAVA_HOME&gt;/lib/servicetag/registration.xml"));
 *   registration.storeToXML(out);
 *   out.close();
 * </pre></blockquote>
 * </li> 
 * <li>This example shows how to install service tags that are in the 
 * registration data in the system service tag registry when determined
 * to be available.  The system service tag registry might not have existed 
 * when the registration data was constructed.
 * <br>
 * <blockquote><pre>
 *   if (Registry.isSupported()) {
 *       Set&lt;ServiceTag&gt; svctags = registration.getServiceTags();
 *       for (ServiceTag st : svctags) {
 *           Registry.getSystemRegistry().addServiceTag(st);
 *       }
 *   }
 * </pre></blockquote>
 * </li>
 * </ol>
 * 
 * @see <a href="https://sunconnection.sun.com/inventory">Sun Connection Inventory Channel</a>
 */
public class RegistrationData {
    private final Map<String, String> environment;
    private final Map<String, String> cpuInfo;
    private final Map<String, ServiceTag> svcTagMap;
    private final String urn;

    /**
     * Creates a {@code RegistrationData} object with a generated
     * {@link #getRegistrationURN registration URN}.
     * The following keys in the {@link #getEnvironmentMap environment map}
     * will be initialized for the configuration of the 
     * running system:
     * <blockquote>
     * <tt>hostname</tt>, <tt>osName</tt>,  <tt>osVersion</tt> and
     * <tt>osArchitecture</tt>
     * </blockquote>
     * and the value of other keys may be empty.
     */
    public RegistrationData() {
        this(Util.generateURN());
    }
    
    // package private
    RegistrationData(String urn) {
        this.urn = urn;
        SystemEnvironment sysEnv = SystemEnvironment.getSystemEnvironment();
        this.environment = initEnvironment(sysEnv);
        this.cpuInfo = initCpuInfo(sysEnv);
        this.svcTagMap = new LinkedHashMap<String, ServiceTag>();
    }
    
    private Map<String, String> initEnvironment(SystemEnvironment sysEnv) {
        Map<String, String> map = new LinkedHashMap<String, String>();
        map.put(ST_NODE_HOSTNAME, sysEnv.getHostname());
        map.put(ST_NODE_HOST_ID, sysEnv.getHostId());
        map.put(ST_NODE_OS_NAME, sysEnv.getOsName());
        map.put(ST_NODE_OS_VERSION, sysEnv.getOsVersion());
        map.put(ST_NODE_OS_ARCH, sysEnv.getOsArchitecture());
        map.put(ST_NODE_SYSTEM_MODEL, sysEnv.getSystemModel());
        map.put(ST_NODE_SYSTEM_MANUFACTURER, sysEnv.getSystemManufacturer());
        map.put(ST_NODE_CPU_MANUFACTURER, sysEnv.getCpuManufacturer());
        map.put(ST_NODE_SERIAL_NUMBER, sysEnv.getSerialNumber());
        map.put(ST_NODE_PHYS_MEM, sysEnv.getPhysMem());
        return map;
    }  

    private Map<String, String> initCpuInfo(SystemEnvironment sysEnv) {
        Map<String, String> map = new LinkedHashMap<String, String>();
        map.put(ST_NODE_SOCKETS, sysEnv.getSockets());
        map.put(ST_NODE_CORES, sysEnv.getCores());
        map.put(ST_NODE_VIRT_CPUS, sysEnv.getVirtCpus());
        map.put(ST_NODE_CPU_NAME, sysEnv.getCpuName());
        map.put(ST_NODE_CLOCK_RATE, sysEnv.getClockRate());
        return map;
    }  
    
    /**
     * Returns the uniform resource name of this registration data
     * in this format:
     * <tt>urn:st:&lt;32-char {@link java.util.UUID uuid}&gt;</tt>
     * 
     * @return the URN of this registration data.
     */
    public String getRegistrationURN() {
        return urn;
    }

    /**
     * Returns a map containing the environment information for this
     * registration data.  See the set of <a href="#EnvMap">keys</a>
     * in the environment map. Subsequent update to the environment
     * map via the {@link #setEnvironment setEnvironment} method will not be reflected
     * in the returned map.
     * 
     * @return an environment map for this registration data. 
     */
    public Map<String, String> getEnvironmentMap() {
        return new LinkedHashMap<String,String>(environment);
    }

    /**
     * Returns a map containing the cpu information for this
     * registration data.  Subsequent update to the cpu info
     * map via the {@link #setCpuInfo setCpuInfo} method will not be reflected
     * in the returned map.
     *
     * @return a cpu info map for this registration data.
     */
    public Map<String, String> getCpuInfoMap() {
        return new LinkedHashMap<String,String>(cpuInfo);
    }

    /**
     * Sets an element of the specified {@code name} in the environment map 
     * with the given {@code value}.
     * 
     * @throws IllegalArgumentException if {@code name} is not a valid key
     * in the environment map, or {@code value} is not valid.
     */
    public void setEnvironment(String name, String value) {    
        if (name == null) {
            throw new NullPointerException("name is null");
        }
        if (value == null) {
            throw new NullPointerException("value is null");
        }
        if (environment.containsKey(name)) {
            if (name.equals(ST_NODE_HOSTNAME) || name.equals(ST_NODE_OS_NAME)) {
                if (value.length() == 0) {
                    throw new IllegalArgumentException("\"" +
                        name + "\" requires non-empty value.");
                }
            }
            environment.put(name, value);
        } else {
            throw new IllegalArgumentException("\"" +
                 name + "\" is not an environment element.");
        }   
    }

    /**
     * Sets an element of the specified {@code name} in the cpu info map
     * with the given {@code value}.
     *
     * @throws IllegalArgumentException if {@code name} is not a valid key
     * in the cpu info map, or {@code value} is not valid.
     */
    public void setCpuInfo(String name, String value) {    
        if (name == null) {
            throw new NullPointerException("name is null");
        }
        if (value == null) {
            throw new NullPointerException("value is null");
        }
        if (cpuInfo.containsKey(name)) {
            cpuInfo.put(name, value);
        } else {
            throw new IllegalArgumentException("\"" +
                 name + "\" is not an cpuinfo element.");
        }   
    }

    /**
     * Returns all service tags in this registration data.
     * 
     * @return  a {@link Set Set} of the service tags
     * in this registration data.
     */
    public Set<ServiceTag> getServiceTags() {
        return new HashSet<ServiceTag>(svcTagMap.values());
    }

    /**
     * Adds a service tag to this registration data. 
     * If the given service tag has an empty <tt>instance_urn</tt>,
     * this method will generate a URN and place it in the copy 
     * of the service tag in this registration data. 
     * This method will return the {@code ServiceTag} object
     * added to this registration data.
     * 
     * @param st {@code ServiceTag} object to be added.
     * @return a {@code ServiceTag} object added to this registration data.
     * 
     * @throws IllegalArgumentException if 
     *   a service tag of the same {@link ServiceTag#getInstanceURN 
     *   <tt>instance_urn</tt>} already exists in the registry.
     */
    public synchronized ServiceTag addServiceTag(ServiceTag st) {
        ServiceTag svcTag = ServiceTag.newInstanceWithUrnTimestamp(st);
        
        String instanceURN = svcTag.getInstanceURN();
        if (svcTagMap.containsKey(instanceURN)) {
            throw new IllegalArgumentException("Instance_urn = " + instanceURN +
                    " already exists in the registration data.");
        } else {
            svcTagMap.put(instanceURN, svcTag);
        }
        return svcTag;
    }

    /**
     * Returns a service tag of the given <tt>instance_urn</tt> in this registration
     * data.
     *
     * @param instanceURN the <tt>instance_urn</tt> of the service tag
     * @return the {@code ServiceTag} object of the given  <tt>instance_urn</tt>
     *         if exists; otherwise return {@code null}.
     */
    public synchronized ServiceTag getServiceTag(String instanceURN) {
        if (instanceURN == null) {
            throw new NullPointerException("instanceURN is null");
        }
        return svcTagMap.get(instanceURN);
    }

    /**
     * Removes a service tag of the given <tt>instance_urn</tt> from this 
     * registration data.
     *
     * @param instanceURN the <tt>instance_urn</tt> of 
     * the service tag to be removed.
     *
     * @return the removed {@code ServiceTag} object;
     * or {@code null} if the service tag does not exist in this 
     * registration data.
     */
    public synchronized ServiceTag removeServiceTag(String instanceURN) {
        if (instanceURN == null) {
            throw new NullPointerException("instanceURN is null");
        }

        ServiceTag svcTag = null;
        if (svcTagMap.containsKey(instanceURN)) {
            svcTag = svcTagMap.remove(instanceURN);
        }
        return svcTag;
    }

     /**
     * Updates the <tt>product_defined_instance_id</tt> in the service tag 
     * of the given <tt>instance_urn</tt> in this registration data.
     * 
     * @param instanceURN the <tt>instance_urn</tt> of the service tag to be updated.
     * @param productDefinedInstanceID the value of the
     * <tt>product_defined_instance_id</tt> to be set.
     * 
     * @return the updated {@code ServiceTag} object;
     * or {@code null} if the service tag does not exist in this 
     * registration data.
     */
    public synchronized ServiceTag updateServiceTag(String instanceURN,
                                                    String productDefinedInstanceID) {
        ServiceTag svcTag = getServiceTag(instanceURN);
        if (svcTag == null) {
            return null;
        }

        svcTag = ServiceTag.newInstanceWithUrnTimestamp(svcTag);
        // update the product defined instance ID field
        svcTag.setProductDefinedInstanceID(productDefinedInstanceID);
        svcTagMap.put(instanceURN, svcTag);
        return svcTag;
    }
    
    /**
     * Reads the registration data from the XML document on the 
     * specified input stream.  The XML document must be 
     * in the format described by the <a href="#XMLSchema">
     * registration data schema</a>.
     * The specified stream is closed after this method returns.
     * 
     * @param in the input stream from which to read the XML document.
     * @return a {@code RegistrationData} object read from the input
     * stream.
     * 
     * @throws IllegalArgumentException if the input stream 
     * contains an invalid registration data.
     * 
     * @throws IOException if an error occurred when reading from the input stream.
     */
    public static RegistrationData loadFromXML(InputStream in) throws IOException {
        try {
            return RegistrationDocument.load(in);
        } finally {
            in.close();
        }
    }
    
    /**
     * Writes the registration data to the specified output stream 
     * in the format described by the <a href="#XMLSchema">
     * registration data schema</a> with "UTF-8" encoding.
     * The specified stream remains open after this method returns.
     * 
     * @param os the output stream on which to write the XML document.
     * 
     * @throws IOException if an error occurred when writing to the output stream.
     */
    public void storeToXML(OutputStream os) throws IOException {
        RegistrationDocument.store(os, this);
        os.flush();
    }

    /** 
     * Returns a newly allocated byte array containing the registration
     * data in XML format.
     *
     * @return a newly allocated byte array containing the registration
     * data in XML format.
     */
    public byte[] toXML() {
        try {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            storeToXML(out);
            return out.toByteArray();
        } catch (IOException e) {
            // should not reach here
            return new byte[0];
        }
    }

    /**
     * Returns a string representation of this registration data in XML
     * format.
     * 
     * @return a string representation of this registration data in XML
     *         format.
     */
    @Override
    public String toString() {
        try {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            storeToXML(out);
            return out.toString("UTF-8");
        } catch (IOException e) {
            // should not reach here
            return "Error creating the return string.";
        }
    }
}
