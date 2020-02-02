= Uploading LibreOffice API to Maven Central

This file documents the prerequisites and workflow to upload LibreOffice
API to Maven Central or local Maven repository.

To install LibreOffice API to local Maven repository or deploy the API
to the Maven Central, extra build toolchain is required.

`Bazel` build tool is used to build sources and javadocs for the API and
install or deploy the artifacts to Maven repository. `Maven` commands are
invoked for that from within `Bazel` driven build - so make sure you've
Maven installed, too. To be able to upload the API to Maven Central, access
must be granted to LibreOffice project on OSSRH.


== Bazel

`Bazel` is a build tool that uses Python to write build files. It is
maintained by Google and is available under Apache 2 license.


=== Installing Bazel

See link:https://docs.bazel.build/versions/master/install.html[Documentation].


== Preparations to publish LibreOffice API to Maven Central


=== Deploy Configuration settings for Maven Central


To be able to publish artifacts to Maven Central some preparations must
be done:

* Create an account on
link:https://issues.sonatype.org/secure/Signup!default.jspa[Sonatype's Jira].

Sonatype is the company that runs Maven Central and you need a Sonatype
account to be able to upload artifacts to Maven Central.

* Configure your Sonatype user and password in `~/.m2/settings.xml`:

----
<settings>
  <servers>
    <server>
     <id>sonatype-nexus-staging</id>
     <username>USER</username>
     <password>PASSWORD</password>
   </server>
  </servers>
</settings>
----

* Request permissions to upload artifacts to the `org.libreoffice`
repository on Maven Central:

Ask for this permission by adding a comment on the
link:https://issues.sonatype.org/browse/OSSRH-19129[OSSRH-19129] Jira
ticket at Sonatype.

The request needs to be approved by someone who already has this
permission by commenting on the same issue.

* Generate and publish a PGP key

Generate and publish a PGP key as described in
link:http://central.sonatype.org/pages/working-with-pgp-signatures.html[
Working with PGP Signatures].

Please be aware that after publishing your public key it may take a
while until it is visible to the Sonatype server.

The PGP key is needed to be able to sign the artifacts before the
upload to Maven Central.

The PGP passphrase can be put in `~/.m2/settings.xml`, or
alternatively make gpg use the agent to provide and cache the
credentials:

----
<settings>
  <profiles>
    <profile>
      <id>gpg</id>
      <properties>
        <gpg.executable>gpg2</gpg.executable>
        <gpg.passphrase>mypassphrase</gpg.passphrase>
        <gpg.keyname>mykeynameoremail</gpg.keyname>
        <gpg.useAgent>true</gpg.useAgent>
      </properties>
    </profile>
  </profiles>
  <activeProfiles>
    <activeProfile>gpg</activeProfile>
  </activeProfiles>
</settings>
----

It can also be included in the key chain on macOS.


== Update Versions

Before publishing new artifacts to Maven Central, `LIBREOFFICE_VERSION`
in the `version.bzl` file must be updated.

In addition the version must be updated in a number of pom.xml files.

To do this run the `./solenv/bin/version.py` script and provide the new
version as parameter, e.g.:

----
  ./solenv/bin/version.py 6.4.0
----


== Build LibreOffice

Build LibreOffice as usually, so that API JARs are created.


== Publish the LibreOffice artifacts to local Maven repository

Execute this command to install LibreOffice API to your local Maven
repository. For troubleshooting, the environment variable `VERBOSE`
can be set:

----
  VERBOSE=1 solenv/maven/msn.sh install
----

Once executed, the local Maven repository contains the LibreOffice API
artifacts:

----
  $ ls -1 ~/.m2/repository/org/libreoffice/unoil/6.4.0/
  _maven.repositories
  unoil-6.4.0.jar
  unoil-6.4.0-javadoc.jar
  unoil-6.4.0.pom
  unoil-6.4.0-sources.jar
----


== Publish the LibreOffice artifacts to Maven Central

* Make sure you have done the configuration for deploying to Maven Central.
* Make sure that the version is updated in the `VERSION` file and in
the `pom.xml` files as described above.

Push the API to Maven Central:

----
  solenv/maven/mvn.sh deploy
----

For troubleshooting, the environment variable `VERBOSE` can be set. This
prints out the commands that are executed by the Bazel build process:

----
  VERBOSE=1 solenv/maven/mvn.sh deploy
----

* To where the artifacts are uploaded depends on the `LIBREOFFICE_VERSION`
in the `version.bzl` file:

** SNAPSHOT versions are directly uploaded into the Sonatype snapshots
repository and no further action is needed:

https://oss.sonatype.org/content/repositories/snapshots/org/libreoffice/

** Release versions are uploaded into a staging repository in the
link:https://oss.sonatype.org/[Sonatype Nexus Server].

* Verify the staging repository

** Go to the link:https://oss.sonatype.org/[Sonatype Nexus Server] and
sign in with your Sonatype credentials.

** Click on 'Build Promotion' in the left navigation bar under
'Staging Repositories' and find the `orglibreoffice-XXXX` staging
repository.

** Verify its content

While the staging repository is open you can upload further content and
also replace uploaded artifacts. If something is wrong with the staging
repository you can drop it by selecting it and clicking on `Drop`.

** Run Sonatype validations on the staging repository

Select the staging repository and click on `Close`. This runs the
Sonatype validations on the staging repository. The repository will
only be closed if everything is OK. A closed repository cannot be
modified anymore, but you may still drop it if you find any issues.

** Test closed staging repository

Once a repository is closed you can find the URL to it in the `Summary`
section, e.g. https://oss.sonatype.org/content/repositories/orglibreoffice-4711

Use this URL for further testing of the artifacts in this repository,
e.g. to try building an extension against this API in this repository
update the version in the `pom.xml` and configure the repository:

----
  <repositories>
    <repository>
      <id>mexus-staging-repository</id>
      <url>https://oss.sonatype.org/content/repositories/orglibreoffice-4711</url>
    </repository>
  </repositories>
----

* Release the staging repository

How to release a staging repository is described in the
link:https://docs.sonatype.org/display/Repository/Sonatype+OSS+Maven+Repository+Usage+Guide#SonatypeOSSMavenRepositoryUsageGuide-8.a.2.ReleasingaStagingRepository[
Sonatype OSS Maven Repository Usage Guide].

[WARNING]
Releasing artifacts to Maven Central cannot be undone!

** Find the closed staging repository in the
link:https://oss.sonatype.org/[Sonatype Nexus Server], select it and
click on `Release`.

** The released artifacts are available in
https://oss.sonatype.org/content/repositories/releases/org/libreoffice/

** It may take up to 2 hours until the artifacts appear on Maven
Central:

http://central.maven.org/maven2/org/libreoffice/

* [optional]: View download statistics

** Sign in to the
link:https://oss.sonatype.org/[Sonatype Nexus Server].

** Click on 'Views/Repositories' in the left navigation bar under
'Central Statistics'.

** Select `org.libreoffice` as `Project`.
