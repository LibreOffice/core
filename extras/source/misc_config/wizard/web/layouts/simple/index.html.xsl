<?xml version="1.0" encoding="UTF-8"?>
<!-- =================================================

This template is a skeleton for single level TOC pages 
Do not overwrite this ! copy it and complete the missing
code.

I use the @ character whereever there is a missing code, so 
you can use a simple find to navigate and find the
places...

====================================================== -->

<xsl:stylesheet version="1.0" 
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">
	
	<xsl:output method               = "html"
                media-type           = "text/html"
                indent               = "yes"
                doctype-public       = "-//W3C//DTD HTML 4.0 Transitional//EN"
                omit-xml-declaration = "yes"
                standalone           = "yes" />
 
   
   <xsl:include href="../layout.xsl"/>

	 
	 <!-- =============================
	               HTML BODY
	 ================================== -->
	 
	 <xsl:template name="body">
	 	<body>
	 	
	 	   <table width="100%" border="0" cellpadding="0" cellspacing="0" class="tcolor">

		 	<xsl:call-template name="title"/>
		 	<xsl:call-template name="toc"/>
		 	
		 	</table>
		 	
		</body>
	 </xsl:template>
	 
	 
	 <xsl:template name="toc">
		<!--
		     @ pre toc html here
		-->
			<tr> 
    			<td width="30" class="ccolor">&#160;
    			</td>
    			<td>

		<!-- use this alternative if you do not need to use groups 
		(uncomment to use - and do not forget to comment the group 
		option above...)-->

	  <xsl:apply-templates select="/session/content/document"/>

		<!--
		     @ post toc html here
		-->
				</td>
			</tr>

	 </xsl:template>
	 
	<!-- also when using groups, in the end it comes 
	to this template, which is called for each document -->
	<xsl:template match="document">
		<p>
			<xsl:apply-templates select="@icon"/>
	
			<xsl:apply-templates select="@title"/>
			<xsl:apply-templates select="@description"/>
			<xsl:apply-templates select="@author"/>
			<xsl:apply-templates select="@create-date"/>
			<xsl:apply-templates select="@update-date"/>
			<xsl:apply-templates select="@filename"/>
			<xsl:apply-templates select="@format"/>
			<xsl:apply-templates select="@format-icon"/>
			<xsl:apply-templates select="@format-icon"/>
		</p>
		
		<xsl:if test="position() &lt; last()">
			<hr/>
		</xsl:if>
		
	</xsl:template>
		
	 <xsl:template name="document-group">
	 	<xsl:param name="group"/>
	 	
	 	<!-- @ pre group code here -->
	 	
	 	<!-- - - -->

		 	<xsl:variable name="count" select="(position() - 1) * $group + 1"/>
		 	
		 	<xsl:for-each select="/session/content/document[$count &lt;= position() and position() &lt; ($count + $group)]">
		 	
					<xsl:apply-templates select="."/>
			 	  
		 	</xsl:for-each>
	 	
	 	<!-- @ post group code here -->
	 	
	 	<!-- - - -->
	 	
	 </xsl:template>

	 	 
	 <xsl:template name="title">
	    <!--
	      @ Pre title html code here
	    -->
  		<tr> 
    		<td height="80" colspan="2" class="toctitle">

				<xsl:value-of select="/session/general-info/@title"/>
	    
	    <!--
	      @ Post title html code here
	    -->
	    	</td>
	    </tr>
	 </xsl:template>
	 
</xsl:stylesheet>
