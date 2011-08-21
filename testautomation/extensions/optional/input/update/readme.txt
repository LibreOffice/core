The folder contains only simple extension. That is, they only contain 
- META-INF
-t.rdb
-description.xml

The description.xml contains only a version and one URL to the update data

For example:


<?xml version="1.0" encoding="UTF-8"?>
<description xmlns="http://openoffice.org/extensions/description/2006"
    xmlns:xlink="http://www.w3.org/1999/xlink">
    <version value="1.0" />    

    <update-information>
      <src xlink:href="http://extensions.openoffice.org/testarea/desktop/simple/plain1.update.xml" />
    </update-information>
</description>


The update information which is referenced in the update-information and the update is committed in the extensions/www project. To modify them get the project:

cvs co extensions/wwww

the files can be found under extensions/www/testarea/desktop
