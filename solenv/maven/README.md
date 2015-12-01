= Uploading LibreOffice API to Maven Central

This file documents the prerequisites and workflow to upload LibreOffice
API to Maven Central or local Maven repository.

To install LibreOffice API to local Maven repository or deploy the API
to the Maven Central, extra build toolchain is required.

`Ant` is used to bootstrap `Buck` build tool. `Buck` build tool is used to
build sources and javadocs for the API and install or deploy the artifacts
to Maven repository. `Maven` commands are invoked for that from within
`Buck` driven build. To be able to upload the API to Maven Central, access
must be granted to LibreOffice project on OSSRH.


== Buck

`Buck` is new build tool that uses Python to write build files. It is
maintained by Facebook and is available under Apache 2 license.


=== Installing Buck

There is currently no binary distribution of `Buck`, so it has to be manually
built and installed. Apache Ant and gcc are required.

Clone the git and build it:

----
  git clone https://github.com/facebook/buck
  cd buck
  ant
----

If you don't have a `bin/` directory in your home directory, create one:

----
  mkdir ~/bin
----

Add the `~/bin` folder to the path:

----
  PATH=~/bin:$PATH
----

Note that the buck executable needs to be available in all shell sessions,
so also make sure it is appended to the path globally.

Add a symbolic link in `~/bin` to the buck and buckd executables:

----
  ln -s `pwd`/bin/buck ~/bin/
  ln -s `pwd`/bin/buckd ~/bin/
----

Verify that `buck` is accessible:

----
  which buck
----

To enable autocompletion of buck commands, install the autocompletion
script from `./scripts/buck_completion.bash` in the buck project.  Refer
to the script's header comments for installation instructions.


=== Prerequisites

Buck requires Python version 2.7 to be installed. The Maven download toolchain
requires `curl` to be installed.


=== Using Buck daemon

Buck ships with a daemon command `buckd`, which uses the
link:https://github.com/martylamb/nailgun[Nailgun] protocol for running
Java programs from the command line without incurring the JVM startup
overhead.

Using a Buck daemon can save significant amounts of time as it avoids the
overhead of starting a Java virtual machine, loading the buck class files
and parsing the build files for each command.

It is safe to run several buck daemons started from different project
directories and they will not interfere with each other. Buck's documentation
covers daemon in http://facebook.github.io/buck/command/buckd.html[buckd].

To use `buckd` the additional
link:https://facebook.github.io/watchman[watchman] program must be installed.

To disable `buckd`, the environment variable `NO_BUCKD` must be set. It's not
recommended to put it in the shell config, as it can be forgotten about it and
then assumed Buck was working as it should when it should be using buckd.
Prepend the variable to Buck invocation instead:

----
  NO_BUCKD=1 buck build api
----


=== Installing watchman

Watchman is used internally by Buck to monitor directory trees and is needed
for buck daemon to work properly. Because buckd is activated by default in the
latest version of Buck, it searches for the watchman executable in the
path and issues a warning when it is not found and kills buckd.

To prepare watchman installation on Linux:

----
  git clone https://github.com/facebook/watchman.git
  cd watchman
  ./autogen.sh
----

To install it in user home directory (without root privileges):

----
  ./configure --prefix $HOME/watchman
  make install
----

To install it system wide:

----
  ./configure
  make
  sudo make install
----

Put $HOME/watchman/bin/watchman in path or link to $HOME/bin/watchman.

To install watchman on OS X:

----
  brew install --HEAD watchman
----

See the original documentation for more information:
link:https://facebook.github.io/watchman/docs/install.html[Watchman
installation].


=== Override Buck's settings

Additional JVM args for Buck can be set in `.buckjavaargs` in the
project root directory. For example to override Buck's default 1GB
heap size:

----
  cat > .buckjavaargs <<EOF
  -XX:MaxPermSize=512m -Xms8000m -Xmx16000m
  EOF
----


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
<server>
  <id>sonatype-nexus-staging</id>
  <username>USER</username>
  <password>PASSWORD</password>
</server>
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

The PGP passphrase can be put in `~/.m2/settings.xml`:

----
<settings>
  <profiles>
    <profile>
      <id>gpg</id>
      <properties>
        <gpg.executable>gpg2</gpg.executable>
        <gpg.passphrase>mypassphrase</gpg.passphrase>
      </properties>
    </profile>
  </profiles>
  <activeProfiles>
    <activeProfile>gpg</activeProfile>
  </activeProfiles>
</settings>
----

It can also be included in the key chain on OS X.


== Update Versions

Before publishing new artifacts to Maven Central, `LIBREOFFICE_VERSION`
in the `VERSION` file must be updated, e.g. change it from `5.0.0` to `5.1.0`.

In addition the version must be updated in a number of pom.xml files.

To do this run the `./solenv/bin/version.py` script and provide the new
version as parameter, e.g.:

----
  ./solenv/bin/version.py 5.1.0
----


== Build LibreOffice

Build LibreOffice as usually, so that API JARs are created.


== Publish the LibreOffice artifacts to local Maven repository

Execute this command to install LibreOffice API to local Maven repository:

----
  buck build api_install
----

Once executed, he local Maven respoitory contains the LibreOffice API
artifacts:

----
  $ ls -1 ~/.m2/repository/org/libreoffice/unoil/5.1.0/
  _maven.repositories
  unoil-5.1.0.jar
  unoil-5.1.0-javadoc.jar
  unoil-5.1.0.pom
  unoil-5.1.0-sources.jar
----


== Publish the LibreOffice artifacts to Maven Central

* Make sure you have done the configuration for deploying to Maven Central.
* Make sure that the version is updated in the `VERSION` file and in
the `pom.xml` files as described above.

Push the API to Maven Central:

----
  buck build api_deploy
----

For troubleshooting, the environment variable `VERBOSE` can be set. This
prints out the commands that are executed by the Buck build process:

----
  VERBOSE=1 buck build api_deploy
----

If no artifacts are uploaded, clean the `buck-out` folder and retry:

----
  rm -rf buck-out
----

* To where the artifacts are uploaded depends on the `LIBREOFFICE_VERSION`
in the `VERSION` file:

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
e.g. to try building a extension against this API in this repository
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
