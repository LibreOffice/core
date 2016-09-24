The folder contains only simple extension. That is, they only contain 
- META-INF
-t.rdb
-description.xml

The description.xml contains  a version, a display name, and one URL to the update data

For example:


<?xml version="1.0" encoding="UTF-8"?>
<description xmlns="http://openoffice.org/extensions/description/2006"
    xmlns:xlink="http://www.w3.org/1999/xlink">
    <version value="1.0" />    

    <display-name>
      <name lang="de">plain1 de</name>
    </display-name>

    <update-information>
      <src xlink:href="http://extensions.openoffice.org/testarea/desktop/simple/plain1.update.xml" />
    </update-information>
</description>

There is only one language as display name available, which will therefore always be displayed.

The update information which is referenced in the update-information and the update is committed in the extensions/www project. To modify them get the project:

cvs co extensions/wwww

the files can be found under extensions/www/testarea/desktop
